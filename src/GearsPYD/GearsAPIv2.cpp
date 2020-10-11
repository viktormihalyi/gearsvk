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
    Sequence::P sequence;

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
    VulkanEnvironment&               environment;
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
        window = nullptr;
    }
};

static SequenceAdapterU currentSeq = nullptr;


void InitializeEnvironment ()
{
    GetVkEnvironment ();
}


void DestroyEnvironment ()
{
    currentSeq.reset ();
    DestroyVkEnvironment ();
}


void SetRenderGraphFromSequence (Sequence::P seq)
{
    currentSeq = SequenceAdapter::Create (GetVkEnvironment (), seq);
#if 0
    try {
        GVK_ASSERT_THROW (env != nullptr);

        currentSequence = seq;
        stimulusToGraphIndex.clear ();
        graphs.clear ();
        reflections.clear ();
        passToOperation.clear ();

        for (auto [startFrame, stim] : seq->getStimuli ()) {
            RenderGraphP renderGraph = RenderGraph::CreateShared ();

            SwapchainImageResourceP presented = renderGraph->CreateResource<SwapchainImageResource> (*env);

            std::map<Pass::P, GearsVk::UUID> newMapping;

            auto passes = stim->getPasses ();
            for (auto pass : passes) {
                auto vert = pass->getStimulusGeneratorVertexShaderSource (Pass::RasterizationMode::fullscreen);
                auto frag = pass->getStimulusGeneratorShaderSource ();

                auto seqpip = ShaderPipeline::CreateShared (*env->device);

                seqpip->SetVertexShaderFromString (vert);
                seqpip->SetFragmentShaderFromString (frag);

                const auto isEnabled = [&] () -> bool { return true; };

                OperationP passOperation = renderGraph->CreateOperation<ConditionalRenderOperation> (
                    DrawRecordableInfo::CreateShared (1, 4), seqpip, isEnabled, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP);

                renderGraph->CreateOutputConnection (*passOperation, 0, *presented);
                newMapping[pass] = passOperation->GetUUID ();
                break;
            }
            GraphSettings s (*env->deviceExtra, env->swapchain->GetImageCount ());

            auto refl = RG::UniformReflection::CreateShared (*renderGraph);
            RG::CreateEmptyImageResources (*renderGraph);

            renderGraph->Compile (s);

            for (auto pass : stim->getPasses ()) {
                for (auto [name, value] : pass->shaderVariables) {
                    (*refl)[newMapping.at (pass)][ShaderKind::Fragment][std::string ("ubo_" + name)] = static_cast<float> (value);
                }

                for (auto [name, value] : pass->shaderVectors) {
                    (*refl)[newMapping.at (pass)][ShaderKind::Fragment][std::string ("ubo_" + name)] = static_cast<glm::vec2> (value);
                }

                for (auto [name, value] : pass->shaderColors) {
                    (*refl)[newMapping.at (pass)][ShaderKind::Fragment][std::string ("ubo_" + name)] = static_cast<glm::vec3> (value);
                }

                break;
            }

            stimulusToGraphIndex[stim] = graphs.size ();
            graphs.push_back (renderGraph);
            reflections.push_back (refl);
            passToOperation.push_back (newMapping);
        }

        Shader::uniformBoundEvent += [&] (const std::string& asd) {
            std::cout << "uniform \"" << asd << "\" bound" << std::endl;
        };
    } catch (std::exception& e) {
        GVK_BREAK ("msg");
    }
#endif
}


static void RenderFrame_ (Window& window, RG::Renderer& renderer, RG::RenderGraph& graph)
{
    window.PollEvents ();
    renderer.RenderNextFrame (graph);
}


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


void RenderSequence ()
{
}


void StartRendering (const std::function<bool ()>& doRender)
{
    currentSeq->RenderFull ();
}


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


void CreateSurface (intptr_t hwnd)
{
    SurfaceU   s  = Surface::Create (Surface::ForWin32, *GetVkEnvironment ().instance, reinterpret_cast<void*> (hwnd));
    SwapchainU sw = RealSwapchain::Create (*GetVkEnvironment ().physicalDevice, *GetVkEnvironment ().device, *s);
    (void)sw;
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