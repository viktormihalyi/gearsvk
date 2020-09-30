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

WindowU            window;
VulkanEnvironmentU env;

std::map<Stimulus::CP, uint32_t>              stimulusToGraphIndex;
std::vector<RenderGraphP>                     graphs;
std::vector<RG::UniformReflectionP>           refls;
std::vector<std::map<Pass::P, GearsVk::UUID>> mapping;
Sequence::P                                   cseq;

void InitializeEnvironment ()
{
    window = HiddenGLFWWindow::Create (); // create a hidden window by default
    env    = VulkanEnvironment::Create (*window);
}


void DestroyEnvironment ()
{
    refls.clear ();
    graphs.clear ();
    window.reset ();
    env.reset ();
}


void SetRenderGraphFromSequence (Sequence::P seq)
{
    GVK_ASSERT_THROW (env != nullptr);

    cseq = seq;
    stimulusToGraphIndex.clear ();
    graphs.clear ();
    refls.clear ();
    mapping.clear ();

    for (auto [startFrame, stim] : seq->getStimuli ()) {
        RenderGraphP renderGraph = RenderGraph::CreateShared ();

        SwapchainImageResourceP presented = renderGraph->CreateResource<SwapchainImageResource> (*env->swapchain);

        std::map<Pass::P, GearsVk::UUID> newMapping;

        auto passes = stim->getPasses ();
        for (auto pass : passes) {
            auto vert = pass->getStimulusGeneratorVertexShaderSource (Pass::RasterizationMode::fullscreen);
            auto frag = pass->getStimulusGeneratorShaderSource ();

            auto seqpip = ShaderPipeline::CreateShared (*env->device);

            seqpip->SetVertexShaderFromString (vert);
            seqpip->SetFragmentShaderFromString (frag);

            const auto isEnabled = [&] () -> bool { return true; };

            OperationP redFillOperation = renderGraph->CreateOperation<ConditionalRenderOperation> (
                DrawRecordableInfo::CreateShared (1, 4), seqpip, isEnabled, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP);

            renderGraph->CreateOutputConnection (*redFillOperation, 0, *presented);
            newMapping[pass] = redFillOperation->GetUUID ();
            break;
        }
        GraphSettings s (*env->deviceExtra, *env->swapchain);

        auto           refl = RG::UniformReflection::CreateShared (*renderGraph, s);
        RG::ImageAdder img (*renderGraph, s);

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
        refls.push_back (refl);
        mapping.push_back (newMapping);
    }

    Shader::uniformBoundEvent += [&] (const std::string& asd) {
        std::cout << "uniform \"" << asd << "\" bound" << std::endl;
    };
}


void StartRendering (const std::function<bool ()>& doRender)
{
    GVK_ASSERT_THROW (env != nullptr);

    env->Wait ();

    window->Show ();

    GraphSettings s (*env->deviceExtra, *env->swapchain);

    SynchronizedSwapchainGraphRenderer swapchainSync (s, *env->swapchain);

    double lastImageTime = 0;

    swapchainSync.swapchainImageAcquiredEvent += [&] () {
        const double currentTime = TimePoint::SinceApplicationStart ().AsMilliseconds ();

        lastImageTime = currentTime;
    };

    const glm::vec2 patternSizeOnRetina (1920, 1080);

    uint32_t frameCount = 0;

    window->DoEventLoop ([&] (bool&) {
        //const double timeInSeconds = TimePoint::SinceApplicationStart ().AsSeconds ();
        const double timeInSeconds = frameCount / 60.0;

        Stimulus::CP currentStim = cseq->getStimulusAtFrame (frameCount++);

        const uint32_t idx = stimulusToGraphIndex[currentStim];

        RG::UniformReflectionP refl = refls[idx];

        swapchainSync.preSubmitEvent = [&] (RenderGraph& graph, uint32_t frameIndex, uint64_t timeNs) {
            for (auto& [pass, renderOpId] : mapping[idx]) {
                (*refl)[renderOpId][ShaderKind::Vertex]["PatternSizeOnRetina"]       = patternSizeOnRetina;
                (*refl)[renderOpId][ShaderKind::Fragment]["ubo_time"]                = static_cast<float> (timeInSeconds - currentStim->getStartingFrame () / 60.f);
                (*refl)[renderOpId][ShaderKind::Fragment]["ubo_patternSizeOnRetina"] = patternSizeOnRetina;
                (*refl)[renderOpId][ShaderKind::Fragment]["ubo_frame"]               = static_cast<int32_t> (frameCount);
            }

            //refl->PrintDebugInfo ();

            refl->Flush (frameIndex);
        };

        swapchainSync.RenderNextFrame (*graphs[idx]);

        frameCount++;
    });

    window->Hide ();
    window.reset ();

    env->Wait ();
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