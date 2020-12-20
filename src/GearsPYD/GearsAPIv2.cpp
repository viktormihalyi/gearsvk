#include "GearsAPIv2.hpp"

#include "NoInline.hpp"

#include "FullscreenQuad.hpp"
#include "GLFWWindow.hpp"
#include "GraphRenderer.hpp"
#include "Noncopyable.hpp"
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
#include <random>
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

    USING_PTR (StimulusAdapterForPresentable);
    class StimulusAdapterForPresentable : public Noncopyable {
        USING_CREATE (StimulusAdapterForPresentable);

    private:
        const Stimulus::CP stimulus;
        const PresentableP presentable;

        RenderGraphP                        renderGraph;
        RG::UniformReflectionP              reflection;
        std::map<Pass::P, OperationP>       passToOperation;
        SynchronizedSwapchainGraphRendererU renderer;

    public:
        NOINLINE StimulusAdapterForPresentable (const VulkanEnvironment& environment, PresentableP& presentable, const Stimulus::CP& stimulus)
            : stimulus (stimulus)
            , presentable (presentable)
        {
            renderGraph = RenderGraph::CreateShared ();

            SwapchainImageResourceP presented = renderGraph->CreateResource<SwapchainImageResource> (*presentable);

            GraphSettings s (*environment.deviceExtra, presentable->GetSwapchain ().GetImageCount ());

            renderer = SynchronizedSwapchainGraphRenderer::Create (s, presentable->GetSwapchain ());

            const std::vector<Pass::P> passes = stimulus->getPasses ();

            GVK_ASSERT (passes.size () == 1);

            for (const Pass::P& pass : passes) {
                GVK_ASSERT (pass->rasterizationMode == Pass::RasterizationMode::fullscreen);
                const std::string vert = pass->getStimulusGeneratorVertexShaderSource (Pass::RasterizationMode::fullscreen);
                const std::string geom = pass->getStimulusGeneratorGeometryShaderSource (Pass::RasterizationMode::fullscreen);
                const std::string frag = pass->getStimulusGeneratorShaderSource ();

                /*
                ShaderPipelineP randomPip = ShaderPipeline::CreateShared (*environment.device);
                randomPip->SetVertexShaderFromString (*Utils::ReadTextFile (PROJECT_ROOT / "src" / "UserInterface" / "Project" / "Shaders" / "quad.vert"));
                randomPip->SetFragmentShaderFromString (stim->getRandomGeneratorShaderSource ());

                {
                    RG::RenderGraph randomGenerator;

                    auto randomResourceHistory = randomGenerator.CreateResource<SingleWritableImageResource> (VK_FILTER_LINEAR, 256, 256, 5, VK_FORMAT_R8G8B8A8_UINT);
                    auto randomResourceOUT     = randomGenerator.CreateResource<SingleWritableImageResource> (VK_FILTER_LINEAR, 256, 256, 1, VK_FORMAT_R8G8B8A8_UINT);
                    auto randomRender          = randomGenerator.CreateOperation<RenderOperation> (DrawRecordableInfo::CreateShared (1, 4), randomPip, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP);
                    auto backTransfer1         = randomGenerator.CreateOperation<TransferOperation> ();
                    auto backTransfer2         = randomGenerator.CreateOperation<TransferOperation> ();
                    auto backTransfer3         = randomGenerator.CreateOperation<TransferOperation> ();
                    auto backTransfer4         = randomGenerator.CreateOperation<TransferOperation> ();

                    randomGenerator.CreateInputConnection (*randomRender, *randomResourceHistory, ImageInputBinding::Create (0, *randomResourceHistory));
                    randomGenerator.CreateInputConnection (*randomRender, *randomResourceHistory, ImageInputBinding::Create (0, *randomResourceHistory));
                    randomGenerator.CreateOutputConnection (*randomRender, 0, *randomResourceOUT);
                }
                */

                ShaderPipelineP sequencePip = ShaderPipeline::CreateShared (*environment.device);

                sequencePip->SetVertexShaderFromString (vert);
                if (!geom.empty ()) {
                    sequencePip->SetShaderFromSourceString (ShaderKind::Geometry, geom);
                }
                sequencePip->SetFragmentShaderFromString (frag);

                RenderOperationP passOperation = renderGraph->CreateOperation<RenderOperation> (
                    DrawRecordableInfo::CreateShared (1, 4), sequencePip, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP);

                renderGraph->CreateOutputConnection (*passOperation, 0, *presented);
                passToOperation[pass] = passOperation;
                break;
            }

            reflection = RG::UniformReflection::CreateShared (*renderGraph);

            RG::ImageMap imgMap = RG::CreateEmptyImageResources (*renderGraph, [&] (const SR::Sampler& sampler) -> std::optional<CreateParams> {
                if (sampler.name == "gamma") {
                    return std::make_tuple (glm::uvec3 { 256, 0, 0 }, VK_FORMAT_R32_SFLOAT, VK_FILTER_NEAREST);
                }

                if (sampler.name == "randoms") {
                    GVK_ASSERT (stimulus->sequence->maxRandomGridWidth > 0 && stimulus->sequence->maxRandomGridHeight > 0);
                    return std::make_tuple (glm::uvec3 { stimulus->sequence->maxRandomGridWidth, stimulus->sequence->maxRandomGridHeight, 0 }, VK_FORMAT_R32G32B32A32_UINT, VK_FILTER_NEAREST);
                }

                return std::nullopt;
            });

            RG::ReadOnlyImageResourceP randomTexture = imgMap.FindByName ("randoms");
            if (randomTexture != nullptr) {
                randomTexture->Compile (GraphSettings (*environment.deviceExtra, 0));

                std::random_device rd;
                std::mt19937_64    gen (rd ());

                std::uniform_int_distribution<uint32_t> dis;

                const size_t          randomsSize = randomTexture->GetImages ()[0]->GetWidth () * randomTexture->GetImages ()[0]->GetHeight () * 4;
                std::vector<uint32_t> randomValues (randomsSize);
                for (uint32_t i = 0; i < randomsSize; i++)
                    randomValues[i] = dis (gen);
                for (uint32_t i = 0; i < randomTexture->GetImages ()[0]->GetArrayLayers (); i++)
                    randomTexture->CopyLayer (randomValues, i);
            }

            RG::ReadOnlyImageResourceP gammaTexture = imgMap.FindByName ("gamma");
            GVK_ASSERT (gammaTexture != nullptr);

            // this is a one time compile resource, which doesnt use framesinflight attrib
            gammaTexture->Compile (GraphSettings (*environment.deviceExtra, 0));

            std::vector<float> gammaAndTemporalWeights (256, 0.f);
            for (int i = 0; i < 101; i++)
                gammaAndTemporalWeights[i] = stimulus->gamma[i];
            for (int i = 0; i < 64; i++)
                gammaAndTemporalWeights[128 + i] = stimulus->temporalWeights[i];
            gammaTexture->CopyTransitionTransfer (gammaAndTemporalWeights);


            renderGraph->Compile (s);

            SetConstantUniforms ();
        }

    private:
        NOINLINE void SetConstantUniforms ()
        {
            for (auto& [pass, op] : passToOperation) {
                for (auto& [name, value] : pass->shaderVariables)
                    (*reflection)[op->GetUUID ()][ShaderKind::Fragment][std::string ("ubo_" + name)] = static_cast<float> (value);
                for (auto& [name, value] : pass->shaderVectors)
                    (*reflection)[op->GetUUID ()][ShaderKind::Fragment][std::string ("ubo_" + name)] = static_cast<glm::vec2> (value);
                for (auto& [name, value] : pass->shaderColors)
                    (*reflection)[op->GetUUID ()][ShaderKind::Fragment][std::string ("ubo_" + name)] = static_cast<glm::vec3> (value);
            }
        }

        void SetUniforms (const GearsVk::UUID& renderOperationId, const uint32_t frameIndex)
        {
            UniformReflection::ShaderUniforms& vertexShaderUniforms   = (*reflection)[renderOperationId][ShaderKind::Vertex];
            UniformReflection::ShaderUniforms& fragmentShaderUniforms = (*reflection)[renderOperationId][ShaderKind::Fragment];

            const double timeInSeconds = frameIndex / 60.0;

            const glm::vec2 patternSizeOnRetina (1920, 1080);

            vertexShaderUniforms["PatternSizeOnRetina"] = patternSizeOnRetina;

            fragmentShaderUniforms["ubo_time"]                = static_cast<float> (timeInSeconds - stimulus->getStartingFrame () / 60.f);
            fragmentShaderUniforms["ubo_patternSizeOnRetina"] = patternSizeOnRetina;
            fragmentShaderUniforms["ubo_frame"]               = static_cast<int32_t> (frameIndex);

            fragmentShaderUniforms["ubo_swizzleForFft"] = 0xffffffff;

            if (stimulus->sequence->maxRandomGridWidth > 0 && stimulus->sequence->maxRandomGridHeight > 0) {
                fragmentShaderUniforms["ubo_cellSize"] = static_cast<glm::vec2> (
                    stimulus->sequence->fieldWidth_um / stimulus->randomGridWidth,
                    stimulus->sequence->fieldHeight_um / stimulus->randomGridHeight);
                fragmentShaderUniforms["ubo_randomGridSize"] = static_cast<glm::ivec2> (
                    stimulus->sequence->maxRandomGridWidth,
                    stimulus->sequence->maxRandomGridHeight);
            }

            if (stimulus->doesToneMappingInStimulusGenerator) {
                fragmentShaderUniforms["ubo_toneRangeMin"] = stimulus->toneRangeMin;
                fragmentShaderUniforms["ubo_toneRangeMax"] = stimulus->toneRangeMax;

                if (stimulus->toneMappingMode == Stimulus::ToneMappingMode::ERF) {
                    fragmentShaderUniforms["ubo_toneRangeMean"] = stimulus->toneRangeMean;
                    fragmentShaderUniforms["ubo_toneRangeVar"]  = stimulus->toneRangeVar;
                } else {
                    fragmentShaderUniforms["ubo_toneRangeMean"] = 0.f;
                    fragmentShaderUniforms["ubo_toneRangeVar"]  = -1.f;
                }

                fragmentShaderUniforms["ubo_doTone"]           = static_cast<int32_t> (!stimulus->doesDynamicToneMapping);
                fragmentShaderUniforms["ubo_doGamma"]          = static_cast<int32_t> (!stimulus->doesDynamicToneMapping);
                fragmentShaderUniforms["ubo_gammaSampleCount"] = static_cast<int32_t> (stimulus->gammaSamplesCount);
            }
        }

    public:
        NOINLINE void RenderFrameIndex (const uint32_t frameIndex)
        {
            const uint32_t stimulusStartingFrame = stimulus->getStartingFrame ();
            const uint32_t stimulusEndingFrame   = stimulus->getStartingFrame () + stimulus->getDuration ();

            if (GVK_ERROR (frameIndex < stimulusStartingFrame || frameIndex >= stimulusEndingFrame)) {
                return;
            }

            renderer->preSubmitEvent = [&] (RenderGraph& graph, uint32_t swapchainImageIndex, uint64_t timeNs) {
                for (auto& [pass, renderOp] : passToOperation) {
                    SetUniforms (renderOp->GetUUID (), frameIndex);
                }

                reflection->PrintDebugInfo ();

                reflection->Flush (swapchainImageIndex);
            };

            renderer->RenderNextFrame (*renderGraph);
        }
    };

    USING_PTR (StimulusAdapterView);
    class StimulusAdapterView : public Noncopyable {
        USING_CREATE (StimulusAdapterView);

    private:
        VulkanEnvironment& environment;
        const Stimulus::CP stimulus;

        std::map<PresentableP, StimulusAdapterForPresentableP> compiledAdapters;

    public:
        NOINLINE StimulusAdapterView (VulkanEnvironment& environment, const Stimulus::CP& stimulus)
            : environment (environment)
            , stimulus (stimulus)
        {
        }

        NOINLINE void CreateForPresentable (PresentableP& presentable)
        {
            compiledAdapters[presentable] = StimulusAdapterForPresentable::Create (environment, presentable, stimulus);
        }

        NOINLINE void RenderFrameIndex (PresentableP& presentable, const uint32_t frameIndex)
        {
            if (GVK_ERROR (compiledAdapters.find (presentable) == compiledAdapters.end ())) {
                return;
            }

            compiledAdapters[presentable]->RenderFrameIndex (frameIndex);
        }

        uint32_t GetStartingFrame () const
        {
            return stimulus->getStartingFrame ();
        }

        uint32_t GetEndingFrame () const
        {
            return stimulus->getStartingFrame () + stimulus->getDuration ();
        }
    };

    VulkanEnvironment& environment;
    PresentableP       currentPresentable;

    std::map<Stimulus::CP, StimulusAdapterViewP> views;

public:
    NOINLINE SequenceAdapter (VulkanEnvironment& environment, const Sequence::P& sequence)
        : sequence (sequence)
        , environment (environment)
    {
        for (auto& [startFrame, stim] : sequence->getStimuli ()) {
            views[stim] = StimulusAdapterView::Create (environment, stim);
        }
    }

    NOINLINE void RenderFrameIndex (const uint32_t frameIndex)
    {
        auto stim = sequence->getStimulusAtFrame (frameIndex);
        if (stim) {
            views[stim]->RenderFrameIndex (currentPresentable, frameIndex);
        }
    }

    NOINLINE void SetCurrentPresentable (PresentableP presentable)
    {
        currentPresentable = presentable;

        for (auto& [stim, view] : views) {
            view->CreateForPresentable (currentPresentable);
        }
    }

    NOINLINE void RenderFullOnExternalWindow ()
    {
        WindowU window = HiddenGLFWWindow::Create ();

        SetCurrentPresentable (environment.CreatePresentable (*window));

        window->Show ();

        uint32_t frameIndex = 0;

        window->DoEventLoop ([&] (bool& shouldStop) {
            RenderFrameIndex (frameIndex);

            frameIndex++;

            if (frameIndex == sequence->getDuration ()) {
                shouldStop = true;
            }
        });

        window->Close ();
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
    currentSeq->RenderFullOnExternalWindow ();
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


static PresentableP GetSurfaceFromHandle (intptr_t surfaceHandle)
{
    for (const PresentableP& createdSurface : createdSurfaces) {
        if (reinterpret_cast<intptr_t> (createdSurface.get ()) == surfaceHandle) {
            return createdSurface;
        }
    }
    return nullptr;
}


/* exported to .pyd */
void SetCurrentSurface (intptr_t surfaceHandle)
{
    PresentableP presentable = GetSurfaceFromHandle (surfaceHandle);
    if (GVK_ERROR (presentable == nullptr)) {
        return;
    }

    if (GVK_ERROR (currentSeq == nullptr)) {
        return;
    }

    currentSeq->SetCurrentPresentable (presentable);
}


/* exported to .pyd */
void RenderFrame (uint32_t frameIndex)
{
    currentSeq->RenderFrameIndex (frameIndex);
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

#include <pybind11/embed.h>


void SetRenderGraphFromPyxFileSequence (const std::filesystem::path& filePath)
{
    pybind11::scoped_interpreter guard;

    pybind11::exec (R"(
    import os
    import importlib.machinery
    import time
    import Gears as gears
    import AppData

    AppData.initConfigParams()

    importlib.machinery.SourceFileLoader("my_module", "C:/Dev/gearsvk/src/UserInterface/Project/Sequences/stock.py").load_module()
    importlib.machinery.SourceFileLoader("my_module", "C:/Dev/gearsvk/src/UserInterface/Project/Sequences/config.py").load_module()
    importlib.machinery.SourceFileLoader("my_module", "C:/Dev/gearsvk/src/UserInterface/Project/Sequences/DefaultSequence.py").load_module()

    my_module = importlib.machinery.SourceFileLoader("my_module", "./Project/Sequences/4_MovingShapes/1_Bars/04_velocity400.pyx").load_module()

    movingbar = my_module.create(None)

    gears.setSequence(movingbar)
    )");
}