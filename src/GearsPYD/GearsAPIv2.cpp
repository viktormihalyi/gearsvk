#include "GearsAPIv2.hpp"

#include "FullscreenQuad.hpp"
#include "GLFWWindow.hpp"
#include "GraphRenderer.hpp"
#include "Persistent.hpp"
#include "RenderGraph.hpp"
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
RenderGraphU       renderGraph;


void InitializeEnvironment ()
{
    window = HiddenGLFWWindow::Create (); // create a hidden window by default
    env    = VulkanEnvironment::Create (*window);
}


void DestroyEnvironment ()
{
    renderGraph.reset ();
    env.reset ();
    window.reset ();
}

UniformReflectionResourceP global_refl    = nullptr;
Pass*                      glob_firstPass = nullptr;

void SetRenderGraphFromSequence (Sequence::P seq)
{
    GVK_ASSERT_THROW (env != nullptr);

    Stimulus::CP stim = seq->getStimulusAtFrame (61);

    auto passes    = stim->getPasses ();
    auto firstPass = passes[0];
    glob_firstPass = firstPass.get ();
    auto vert      = firstPass->getStimulusGeneratorVertexShaderSource (Pass::RasterizationMode::fullscreen);
    auto frag      = firstPass->getStimulusGeneratorShaderSource ();

    std::cout << " ========================= fragment shader BEGIN ========================= " << std::endl;
    std::cout << frag << std::endl;
    std::cout << " ========================= fragment shader END =========================== " << std::endl;

    auto seqpip = ShaderPipeline::CreateShared (*env->device);

    seqpip->SetVertexShaderFromString (vert);
    seqpip->SetFragmentShaderFromString (frag);

    renderGraph = RenderGraph::Create ();


    GraphSettings s (*env->deviceExtra, *env->swapchain);

    SwapchainImageResourceP presented = renderGraph->CreateResource<SwapchainImageResource> (*env->swapchain);
    // ImageResource&             writ      = renderGraph->CreateResource<WritableImageResource> ();
    UniformReflectionResourceP refl = renderGraph->CreateResource<UniformReflectionResource> (seqpip);
    global_refl                     = refl;

    OperationP redFillOperation = renderGraph->AddOperation (RenderOperation::Create (DrawRecordableInfo::CreateShared (1, 4), seqpip, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP));

    renderGraph->CreateOutputConnection (*redFillOperation, 0, *presented);
    // renderGraph->CreateOutputConnection (redFillOperation, 1, writ);

    for (uint32_t i = 0; i < refl->uboRes.size (); ++i) {
        renderGraph->CreateInputConnection (*redFillOperation, *refl->uboRes[i], UniformInputBinding::Create (refl->bindings[i], *refl->uboRes[i]));
    }

    for (uint32_t i = 0; i < refl->sampledImages.size (); ++i) {
        renderGraph->CreateInputConnection (*redFillOperation, *refl->sampledImages[i], ImageInputBinding::Create (refl->samplerBindings[i], *refl->sampledImages[i]));
    }

    renderGraph->Compile (s);

    Shader::uniformBoundEvent += [&] (const std::string& asd) {
        std::cout << "uniform \"" << asd << "\" bound" << std::endl;
    };
}


void StartRendering (const std::function<bool ()>& doRender)
{
    GVK_ASSERT_THROW (env != nullptr);
    GVK_ASSERT_THROW (renderGraph != nullptr);

    env->Wait ();

    SynchronizedSwapchainGraphRenderer swapchainSync (*renderGraph, *env->swapchain);

    const glm::vec2 patternSizeOnRetina (1920, 1080);

    for (auto [name, value] : glob_firstPass->shaderVariables) {
        global_refl->frag[std::string ("ubo_" + name)]["value"] = static_cast<float> (value);
    }

    for (auto [name, value] : glob_firstPass->shaderVectors) {
        global_refl->frag[std::string ("ubo_" + name)]["value"] = static_cast<glm::vec2> (value);
    }

    for (auto [name, value] : glob_firstPass->shaderColors) {
        global_refl->frag[std::string ("ubo_" + name)]["value"] = static_cast<glm::vec3> (value);
    }

    uint32_t frameCount = 0;
    swapchainSync.preSubmitEvent += [&] (uint32_t frameIndex, uint64_t timeNs) {
        global_refl->vert["PatternSizeOnRetina"] = patternSizeOnRetina;

        global_refl->frag["ubo_time"] = static_cast<float> (TimePoint::SinceApplicationStart ().AsSeconds ());

        global_refl->Update (frameIndex);
    };

    window->Show ();

    window->DoEventLoop (swapchainSync.GetConditionalDrawCallback (doRender));

    window->Hide ();
    window.reset ();

    env->Wait ();

    {
        Utils::DebugTimerLogger tl ("switching to new window");
        Utils::TimerScope       ts (tl);

        window = HiddenGLFWWindow::Create ();

        env->physicalDevice->RecreateForSurface (window->GetSurface (*env->instance));

        env->swapchain->RecreateForSurface (window->GetSurface (*env->instance));

        GraphSettings s (*env->deviceExtra, *env->swapchain);

        renderGraph->CompileResources (s);
        renderGraph->Compile (s);
    }
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