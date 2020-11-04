#include "GearsAPIv2.hpp"

#include "FullscreenQuad.hpp"
#include "GLFWWindow.hpp"
#include "GraphRenderer.hpp"
#include "Persistent.hpp"
#include "RenderGraph.hpp"
#include "UUID.hpp"
#include "UniformReflection.hpp"
#include "VulkanEnvironment.hpp"
#include "core/Sequence.h"

#include "gpu/Shader.hpp"

#include <algorithm>
#include <atomic>
#include <memory>
#include <set>
#include <thread>
#include <type_traits>

using namespace RG;


class CounterPreprocessor : public ShaderPreprocessor {
public:
    virtual std::string Preprocess (const std::string& source) override
    {
        if (Utils::StringContains (source, "__COUNTER__")) {
            uint32_t counter = 0;
            return Utils::ReplaceAll (source, "__COUNTER__", [&] () -> std::string {
                return std::to_string (counter++);
            });
        }
    }
};


USING_PTR (SequenceAdapter);
class SequenceAdapter {
    USING_CREATE (SequenceAdapter);

private:
    const Sequence::P sequence;

    USING_PTR (StimulusV2);
    class StimulusV2 : public Noncopyable {
        USING_CREATE (StimulusV2);

    public:
        RenderGraphP                  graph;
        RG::UniformReflectionP        reflection;
        std::map<Pass::P, OperationP> passToOperation;

        StimulusV2 (RenderGraphP                  graph,
                    RG::UniformReflectionP        reflection,
                    std::map<Pass::P, OperationP> passToOperation)
            : graph (graph)
            , reflection (reflection)
            , passToOperation (passToOperation)
        {
        }
    };

    std::vector<U<StimulusV2>>       stimulii;
    std::map<Stimulus::CP, uint32_t> stimulusToGraphIndex;
    const VulkanEnvironment&         environment;
    const PresentableP               presentable;

public:
    SequenceAdapter (VulkanEnvironment& environment, const Sequence::P& sequence)
        : sequence (sequence)
        , environment (environment)
        , presentable (Presentable::CreateShared ())
    {
        for (auto& [startFrame, stim] : sequence->getStimuli ()) {
            RenderGraphP renderGraph = RenderGraph::CreateShared ();

            SwapchainImageResourceP presented = renderGraph->CreateResource<SwapchainImageResource> (*presentable);

            std::map<Pass::P, OperationP> newMapping;

            const std::vector<Pass::P> passes = stim->getPasses ();
            for (const Pass::P& pass : passes) {
                GVK_ASSERT (pass->rasterizationMode == Pass::RasterizationMode::fullscreen);
                const std::string vert = pass->getStimulusGeneratorVertexShaderSource (Pass::RasterizationMode::fullscreen);
                const std::string geom = pass->getStimulusGeneratorGeometryShaderSource (Pass::RasterizationMode::fullscreen);
                const std::string frag = pass->getStimulusGeneratorShaderSource ();

                ShaderPipelineP randomPip = ShaderPipeline::CreateShared (*environment.device);
                randomPip->SetVertexShaderFromString (*Utils::ReadTextFile (PROJECT_ROOT / "src" / "UserInterface" / "Project" / "Shaders" / "quad.vert"));
                randomPip->SetFragmentShaderFromString (stim->getRandomGeneratorShaderSource ());

                RG::RenderGraph randomGenerator;
                auto            randomRender   = randomGenerator.CreateOperation<RenderOperation> (DrawRecordableInfo::CreateShared (1, 4), randomPip, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP);
                auto            randomResource = randomGenerator.CreateResource<SingleWritableImageResource> (VK_FILTER_LINEAR, 256, 256, 1, VK_FORMAT_R8G8B8A8_UINT);
                randomGenerator.CreateOutputConnection (*randomRender, 0, *randomResource);
                randomGenerator.Compile (GraphSettings (*environment.deviceExtra, 1));

                ShaderPipelineP sequencePip = ShaderPipeline::CreateShared (*environment.device);

                sequencePip->SetVertexShaderFromString (vert);
                if (!geom.empty ()) {
                    sequencePip->SetShaderFromSourceString (ShaderKind::Geometry, geom);
                }
                sequencePip->SetFragmentShaderFromString (frag);

                RenderOperationP passOperation = renderGraph->CreateOperation<RenderOperation> (
                    DrawRecordableInfo::CreateShared (1, 4), sequencePip, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP);

                renderGraph->CreateOutputConnection (*passOperation, 0, *presented);
                newMapping[pass] = passOperation;
                break;
            }


            RG::UniformReflectionP refl = RG::UniformReflection::CreateShared (*renderGraph);

            RG::ImageMap imgMap = RG::CreateEmptyImageResources (*renderGraph, [&] (const SR::Sampler& sampler) -> std::optional<CreateParams> {
                if (sampler.name == "gamma") {
                    return std::make_tuple (glm::uvec3 { 256, 0, 0 }, VK_FORMAT_R32_SFLOAT, VK_FILTER_NEAREST);
                }

                if (sampler.name == "randoms") {
                    GVK_ASSERT (sequence->maxRandomGridWidth > 0 && sequence->maxRandomGridHeight > 0);
                    return std::make_tuple (glm::uvec3 { sequence->maxRandomGridWidth, sequence->maxRandomGridHeight, 0 }, VK_FORMAT_R32G32B32A32_UINT, VK_FILTER_NEAREST);
                }

                return std::nullopt;
            });

            RG::ReadOnlyImageResourceP gammaTexture = imgMap.FindByName ("gamma");
            GVK_ASSERT (gammaTexture != nullptr);

            // this is a one time compile resource, which doesnt use framesinflight attrib
            gammaTexture->Compile (GraphSettings (*environment.deviceExtra, 0));

            std::vector<float> gammaAndTemporalWeights (256, 0.f);
            for (int i = 0; i < 101; i++)
                gammaAndTemporalWeights[i] = stim->gamma[i];
            for (int i = 0; i < 64; i++)
                gammaAndTemporalWeights[128 + i] = stim->temporalWeights[i];
            gammaTexture->CopyTransitionTransfer (gammaAndTemporalWeights);

            stimulusToGraphIndex[stim] = stimulii.size ();

            stimulii.push_back (StimulusV2::Create (renderGraph, refl, newMapping));
        }
    }

    void RenderFull ()
    {
        WindowU window = HiddenGLFWWindow::Create ();
        *presentable   = std::move (*environment.CreatePresentable (*window));
        window->Show ();
        //window->SetWindowMode (Window::Mode::Fullscreen);

        GraphSettings s (*environment.deviceExtra, presentable->GetSwapchain ().GetImageCount ());

        for (auto& st : stimulii) {
            st->graph->Compile (s);
        }

        for (auto& st : stimulii) {
            for (auto& [pass, op] : st->passToOperation) {
                for (auto& [name, value] : pass->shaderVariables)
                    (*st->reflection)[op->GetUUID ()][ShaderKind::Fragment][std::string ("ubo_" + name)] = static_cast<float> (value);
                for (auto& [name, value] : pass->shaderVectors)
                    (*st->reflection)[op->GetUUID ()][ShaderKind::Fragment][std::string ("ubo_" + name)] = static_cast<glm::vec2> (value);
                for (auto& [name, value] : pass->shaderColors)
                    (*st->reflection)[op->GetUUID ()][ShaderKind::Fragment][std::string ("ubo_" + name)] = static_cast<glm::vec3> (value);
            }
        }

        SynchronizedSwapchainGraphRendererU renderer = SynchronizedSwapchainGraphRenderer::Create (s, presentable->GetSwapchain ());

        const glm::vec2 patternSizeOnRetina (1920, 1080);

        uint32_t frameCount = 0;

        window->DoEventLoop ([&] (bool& shouldStop) {
            //const double timeInSeconds = TimePoint::SinceApplicationStart ().AsSeconds ();
            const double timeInSeconds = frameCount / 60.0;

            Stimulus::CP currentStim = sequence->getStimulusAtFrame (frameCount++);
            if (currentStim == nullptr) {
                shouldStop = true;
                return;
            }

            const uint32_t idx = stimulusToGraphIndex[currentStim];

            RG::UniformReflectionP refl = stimulii[idx]->reflection;

            renderer->preSubmitEvent = [&] (RenderGraph& graph, uint32_t frameIndex, uint64_t timeNs) {
                for (auto& [pass, renderOpId] : stimulii[idx]->passToOperation) {
                    auto& fragmentShaderUniforms = (*refl)[renderOpId->GetUUID ()][ShaderKind::Fragment];

                    (*refl)[renderOpId->GetUUID ()][ShaderKind::Vertex]["PatternSizeOnRetina"] = patternSizeOnRetina;

                    fragmentShaderUniforms["ubo_time"]                = static_cast<float> (timeInSeconds - currentStim->getStartingFrame () / 60.f);
                    fragmentShaderUniforms["ubo_patternSizeOnRetina"] = patternSizeOnRetina;
                    fragmentShaderUniforms["ubo_frame"]               = static_cast<int32_t> (frameCount);

                    fragmentShaderUniforms["ubo_swizzleForFft"] = 0xffffffff;

                    if (currentStim->doesToneMappingInStimulusGenerator) {
                        fragmentShaderUniforms["ubo_toneRangeMin"] = currentStim->toneRangeMin;
                        fragmentShaderUniforms["ubo_toneRangeMax"] = currentStim->toneRangeMax;

                        if (currentStim->toneMappingMode == Stimulus::ToneMappingMode::ERF) {
                            fragmentShaderUniforms["ubo_toneRangeMean"] = currentStim->toneRangeMean;
                            fragmentShaderUniforms["ubo_toneRangeVar"]  = currentStim->toneRangeVar;
                        } else {
                            fragmentShaderUniforms["ubo_toneRangeMean"] = 0.f;
                            fragmentShaderUniforms["ubo_toneRangeVar"]  = -1.f;
                        }

                        fragmentShaderUniforms["ubo_doTone"]           = static_cast<int32_t> (!currentStim->doesDynamicToneMapping);
                        fragmentShaderUniforms["ubo_doGamma"]          = static_cast<int32_t> (!currentStim->doesDynamicToneMapping);
                        fragmentShaderUniforms["ubo_gammaSampleCount"] = static_cast<int32_t> (currentStim->gammaSamplesCount);
                    }
                }


                refl->PrintDebugInfo ();

                refl->Flush (frameIndex);
            };

            renderer->RenderNextFrame (*stimulii[idx]->graph);

            frameCount++;
        });

        window->Close ();
        window.reset ();
        presentable->Clear ();
    }
};

static SequenceAdapterU          currentSeq = nullptr;
static std::vector<PresentableP> createdSurfaces;

static VulkanEnvironment& GetVkEnvironment ();
static void               DestroyVkEnvironment ();


/* exported to .pyd */
void InitializeEnvironment ()
{
    GetVkEnvironment ();
}


/* exported to .pyd */
void DestroyEnvironment ()
{
    createdSurfaces.clear ();
    currentSeq.reset ();
    DestroyVkEnvironment ();
}


void SetRenderGraphFromSequence (Sequence::P seq)
{
    currentSeq = SequenceAdapter::Create (GetVkEnvironment (), seq);
}


static void RenderFrame_ (Window& window, RG::Renderer& renderer, RG::RenderGraph& graph)
{
    window.PollEvents ();
    renderer.RenderNextFrame (graph);
}


/* exported to .pyd */
void RenderFrame (uint32_t frameIndex)
{
}


static void EventLoop (Window& window, RG::Renderer& renderer, RG::RenderGraph& graph, const std::function<bool ()>& stopFn)
{
    try {
        while (true) {
            if (stopFn ()) {
                return;
            }

            RenderFrame_ (window, renderer, graph);
        }
    } catch (std::runtime_error& er) {
    }
}


/* exported to .pyd */
void StartRendering (const std::function<bool ()>& doRender)
{
    currentSeq->RenderFull ();
}


/* exported to .pyd */
void TryCompile (ShaderKind shaderKind, const std::string& source)
{
    try {
        ShaderModule::CreateFromGLSLString (*GetVkEnvironment ().device, shaderKind, source);
        std::cout << "compile succeeded" << std::endl;
    } catch (const ShaderCompileException&) {
        std::cout << "compile failed, source code: " << std::endl;
        std::cout << source << std::endl;
    }
}


/* exported to .pyd */
intptr_t CreateSurface (intptr_t hwnd)
{
#ifdef WIN32
    PresentableP presentable = GetVkEnvironment ().CreatePresentable (Surface::Create (Surface::PlatformSpecific, *GetVkEnvironment ().instance, reinterpret_cast<void*> (hwnd)));
    createdSurfaces.push_back (presentable);
    return reinterpret_cast<intptr_t> (presentable.get ());
#endif
}


/* exported to .pyd */
void DestroySurface (intptr_t surfaceHandle)
{
    createdSurfaces.erase (std::remove_if (createdSurfaces.begin (), createdSurfaces.end (), [&] (const PresentableP& p) {
                               return reinterpret_cast<intptr_t> (p.get ()) == surfaceHandle;
                           }),
                           createdSurfaces.end ());
}


/* exported to .pyd */
void RequestPaint (intptr_t surfaceHandle)
{
    // TODO;
    PresentableP presentable;
    for (PresentableP& p : createdSurfaces) {
        if (reinterpret_cast<intptr_t> (p.get ()) == surfaceHandle) {
            presentable = p;
            break;
        }
    }

    if (GVK_ERROR (presentable == nullptr)) {
        return;
    }

    GraphSettings s (*GetVkEnvironment ().deviceExtra, 3);
    RenderGraph   graph;

    auto sp = ShaderPipeline::CreateShared (*GetVkEnvironment ().deviceExtra);
    sp->SetVertexShaderFromString (R"(
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) out vec2 textureCoords;

vec2 uvs[6] = vec2[] (
    vec2 (0.f, 0.f),
    vec2 (0.f, 1.f),
    vec2 (1.f, 1.f),
    vec2 (1.f, 1.f),
    vec2 (0.f, 0.f),
    vec2 (1.f, 0.f)
);

vec2 positions[6] = vec2[] (
    vec2 (-1.f, -1.f),
    vec2 (-1.f, +1.f),
    vec2 (+1.f, +1.f),
    vec2 (+1.f, +1.f),
    vec2 (-1.f, -1.f),
    vec2 (+1.f, -1.f)
);


void main() {
    gl_Position = vec4 (positions[gl_VertexIndex], 0.0, 1.0);
    textureCoords = uvs[gl_VertexIndex];
}
    )");

    sp->SetFragmentShaderFromString (R"(
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) out vec4 outColor;

void main () {
    outColor = vec4 (1, 0, 0, 1);
}
    )");

    RenderOperationP redFillOperation = graph.CreateOperation<RenderOperation> (DrawRecordableInfo::CreateShared (1, 6), sp);

    ImageResourceP red = graph.CreateResource<SwapchainImageResource> (*presentable);

    graph.CreateOutputConnection (*redFillOperation, 0, *red);

    graph.Compile (s);


    BlockingGraphRenderer renderer (s, presentable->GetSwapchain ());

    renderer.RenderNextFrame (graph);
}


static VulkanEnvironmentU env_ = nullptr;


static VulkanEnvironment& GetVkEnvironment ()
{
    if (env_ == nullptr) {
        env_ = VulkanEnvironment::Create ();
    }
    return *env_;
}


static void DestroyVkEnvironment ()
{
    if (env_ != nullptr) {
        env_.reset ();
    }
}


std::string GetGLSLResourcesForRandoms ()
{
    return R"(
#ifndef GEARS_RANDOMS_RESOURCES
#define GEARS_RANDOMS_RESOURCES
#define RANDOMS_ARRAY_SIZE 5
    layout (binding = 201) uniform usampler2D randoms[RANDOMS_ARRAY_SIZE];
    layout (binding = 202) uniform ubo_cellSize { vec2 cellSize; };
    layout (binding = 203) uniform ubo_randomGridSize { ivec2 randomGridSize; };
    layout (binding = 204) uniform ubo_randomsIndex { uint randomsIndex; };
#endif
    )";
}
