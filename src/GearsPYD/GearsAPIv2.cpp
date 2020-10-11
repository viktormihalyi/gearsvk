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

#include <atomic>
#include <memory>
#include <set>
#include <thread>
#include <type_traits>

using namespace RG;

static VulkanEnvironment& GetVkEnvironment ();
static void               DestroyVkEnvironment ();


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
                const std::string vert = pass->getStimulusGeneratorVertexShaderSource (Pass::RasterizationMode::fullscreen);
                const std::string frag = pass->getStimulusGeneratorShaderSource ();

                ShaderPipelineP sequencePip = ShaderPipeline::CreateShared (*environment.device);

                sequencePip->SetVertexShaderFromString (vert);
                sequencePip->SetFragmentShaderFromString (frag);

                OperationP passOperation = renderGraph->CreateOperation<RenderOperation> (
                    DrawRecordableInfo::CreateShared (1, 4), sequencePip, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP);

                renderGraph->CreateOutputConnection (*passOperation, 0, *presented);
                newMapping[pass] = passOperation;
                break;
            }


            RG::UniformReflectionP refl = RG::UniformReflection::CreateShared (*renderGraph);
            RG::CreateEmptyImageResources (*renderGraph);

            stimulusToGraphIndex[stim] = stimulii.size ();

            stimulii.push_back (StimulusV2::Create (renderGraph, refl, newMapping));
        }
    }

    void RenderFull ()
    {
        WindowU window = HiddenGLFWWindow::Create ();
        *presentable   = std::move (*environment.CreatePresentable (*window));
        window->Show ();

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
                    (*refl)[renderOpId->GetUUID ()][ShaderKind::Vertex]["PatternSizeOnRetina"]       = patternSizeOnRetina;
                    (*refl)[renderOpId->GetUUID ()][ShaderKind::Fragment]["ubo_time"]                = static_cast<float> (timeInSeconds - currentStim->getStartingFrame () / 60.f);
                    (*refl)[renderOpId->GetUUID ()][ShaderKind::Fragment]["ubo_patternSizeOnRetina"] = patternSizeOnRetina;
                    (*refl)[renderOpId->GetUUID ()][ShaderKind::Fragment]["ubo_frame"]               = static_cast<int32_t> (frameCount);
                }

                //refl->PrintDebugInfo ();

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

static SequenceAdapterU currentSeq = nullptr;


/* exported to .pyd */
void InitializeEnvironment ()
{
    GetVkEnvironment ();
}


/* exported to .pyd */
void DestroyEnvironment ()
{
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
void CreateSurface (intptr_t hwnd)
{
#ifdef WIN32
    PresentableP presentable = GetVkEnvironment ().CreatePresentable (Surface::Create (Surface::PlatformSpecific, *GetVkEnvironment ().instance, reinterpret_cast<void*> (hwnd)));
#endif
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
    if (env_ == nullptr) {
        env_.reset ();
    }
}