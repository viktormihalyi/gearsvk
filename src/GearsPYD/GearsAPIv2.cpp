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


static VulkanEnvironmentU env = nullptr;


static void EnsureEnvInitialized ()
{
    if (env == nullptr) {
        env = VulkanEnvironment::Create ();
    }
}


USING_PTR (CurrentSequence);
class CurrentSequence {
    USING_CREATE (CurrentSequence);

private:
    Sequence::P sequence;

    USING_PTR (StimulusV2);
    struct StimulusV2 : public Noncopyable {
        USING_CREATE (StimulusV2);

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
    VulkanEnvironmentU&              env;


public:
    CurrentSequence (VulkanEnvironmentU& env, const Sequence::P& sequence)
        : sequence (sequence)
        , env (env)
    {
        for (auto& [startFrame, stim] : sequence->getStimuli ()) {
            RenderGraphP renderGraph = RenderGraph::CreateShared ();

            SwapchainImageResourceP presented = renderGraph->CreateResource<SwapchainImageResource> (*env);

            std::map<Pass::P, OperationP> newMapping;

            const std::vector<Pass::P> passes = stim->getPasses ();
            for (const Pass::P& pass : passes) {
                const std::string vert = pass->getStimulusGeneratorVertexShaderSource (Pass::RasterizationMode::fullscreen);
                const std::string frag = pass->getStimulusGeneratorShaderSource ();

                ShaderPipelineP sequencePip = ShaderPipeline::CreateShared (*env->device);

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
        window->Show ();

        env->WindowChanged (*window);

        GraphSettings s (*env->deviceExtra, env->swapchain->GetImageCount ());

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

        SynchronizedSwapchainGraphRendererU renderer = SynchronizedSwapchainGraphRenderer::Create (s, *env->swapchain);

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

static CurrentSequenceU currentSeq = nullptr;


void InitializeEnvironment ()
{
    env = VulkanEnvironment::Create ();
}


void DestroyEnvironment ()
{
    currentSeq.reset ();
    env.reset ();
}


void SetRenderGraphFromSequence (Sequence::P seq)
{
    EnsureEnvInitialized ();
    currentSeq = CurrentSequence::Create (env, seq);
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
#if 0
    GVK_ASSERT_THROW (env != nullptr);

    env->Wait ();

    window->Show ();

    GraphSettings s (*env->deviceExtra, env->swapchain->GetImageCount ());

    renderer = SynchronizedSwapchainGraphRenderer::Create (s, *env->swapchain);

    std::vector<double> lastImageTimes;
    uint32_t            lastImageIndex = 0;
    for (uint32_t i = 0; i < env->swapchain->GetImageCount (); ++i) {
        lastImageTimes.push_back (0);
    }

    renderer->swapchainImageAcquiredEvent += [&] (uint32_t imageIndex) {
        const double currentTime = TimePoint::SinceApplicationStart ().AsMilliseconds ();

        const double deltaMs       = currentTime - lastImageTimes[imageIndex];
        const double deltaRenderMs = currentTime - lastImageTimes[lastImageIndex];

        lastImageTimes[imageIndex] = currentTime;
        lastImageIndex             = imageIndex;
    };

    const glm::vec2 patternSizeOnRetina (1920, 1080);

    uint32_t frameCount = 0;

    window->DoEventLoop ([&] (bool& shouldStop) {
        //const double timeInSeconds = TimePoint::SinceApplicationStart ().AsSeconds ();
        const double timeInSeconds = frameCount / 60.0;

        Stimulus::CP currentStim = currentSequence->getStimulusAtFrame (frameCount++);
        if (currentStim == nullptr) {
            shouldStop = true;
            return;
        }

        const uint32_t idx = stimulusToGraphIndex[currentStim];

        RG::UniformReflectionP refl = reflections[idx];

        renderer->preSubmitEvent = [&] (RenderGraph& graph, uint32_t frameIndex, uint64_t timeNs) {
            for (auto& [pass, renderOpId] : passToOperation[idx]) {
                (*refl)[renderOpId][ShaderKind::Vertex]["PatternSizeOnRetina"]       = patternSizeOnRetina;
                (*refl)[renderOpId][ShaderKind::Fragment]["ubo_time"]                = static_cast<float> (timeInSeconds - currentStim->getStartingFrame () / 60.f);
                (*refl)[renderOpId][ShaderKind::Fragment]["ubo_patternSizeOnRetina"] = patternSizeOnRetina;
                (*refl)[renderOpId][ShaderKind::Fragment]["ubo_frame"]               = static_cast<int32_t> (frameCount);
            }

            //refl->PrintDebugInfo ();

            refl->Flush (frameIndex);
        };

        renderer->RenderNextFrame (*graphs[idx]);

        frameCount++;
    });

    window->Hide ();
    window.reset ();

    env->Wait ();
#endif
}

void TryCompile (ShaderKind shaderKind, const std::string& source)
{
    try {
        ShaderModule::CreateFromGLSLString (*env->device, shaderKind, source);
        std::cout << "compile succeeded" << std::endl;
    } catch (const ShaderCompileException&) {
        std::cout << "compile failed, source code: " << std::endl;
        std::cout << source << std::endl;
    }
}