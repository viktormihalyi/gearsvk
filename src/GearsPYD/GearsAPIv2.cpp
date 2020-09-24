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

WindowU                window;
VulkanEnvironmentU     env;
RenderGraphU           renderGraph;
Pass*                  glob_firstPass = nullptr;
RG::UniformReflectionP global_refl;
GearsVk::UUID          renderOpId = nullptr;


void InitializeEnvironment ()
{
    window = HiddenGLFWWindow::Create (); // create a hidden window by default
    env    = VulkanEnvironment::Create (*window);
}


void DestroyEnvironment ()
{
    global_refl.reset ();
    renderGraph.reset ();
    window.reset ();
    env.reset ();
}


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

    std::cout << "> compiling vertex shader" << std::endl;
    seqpip->SetVertexShaderFromString (vert);
    std::cout << "> compiling fragment shader" << std::endl;
    seqpip->SetFragmentShaderFromString (frag);

    renderGraph = RenderGraph::Create ();


    GraphSettings s (*env->deviceExtra, *env->swapchain);

    SwapchainImageResourceP presented        = renderGraph->CreateResource<SwapchainImageResource> (*env->swapchain);
    OperationP              redFillOperation = renderGraph->AddOperation (RenderOperation::Create (DrawRecordableInfo::CreateShared (1, 4), seqpip, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP));

    renderGraph->CreateOutputConnection (*redFillOperation, 0, *presented);

    global_refl = RG::UniformReflection::Create (*renderGraph, s);
    RG::ImageAdder img (*renderGraph, s);

    renderGraph->Compile (s);

    renderOpId = redFillOperation->GetUUID ();

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

    const glm::vec2 patternSizeOnRetina (800, 600);

    for (auto [name, value] : glob_firstPass->shaderVariables) {
        (*global_refl)[renderOpId][ShaderKind::Fragment][std::string ("ubo_" + name)]["value"] = static_cast<float> (value);
    }

    for (auto [name, value] : glob_firstPass->shaderVectors) {
        (*global_refl)[renderOpId][ShaderKind::Fragment][std::string ("ubo_" + name)]["value"] = static_cast<glm::vec2> (value);
    }


    for (auto [name, value] : glob_firstPass->shaderColors) {
        (*global_refl)[renderOpId][ShaderKind::Fragment][std::string ("ubo_" + name)]["value"] = static_cast<glm::vec3> (value);
    }


    uint32_t frameCount = 0;
    swapchainSync.preSubmitEvent += [&] (uint32_t frameIndex, uint64_t timeNs) {
        (*global_refl)[renderOpId][ShaderKind::Vertex]["PatternSizeOnRetina"]["value"] = patternSizeOnRetina;

        (*global_refl)[renderOpId][ShaderKind::Fragment]["ubo_time"]["value"] = static_cast<float> (TimePoint::SinceApplicationStart ().AsSeconds ());
        (*global_refl)[renderOpId][ShaderKind::Fragment]["ubo_patternSizeOnRetina"]["value"] = patternSizeOnRetina;

        global_refl->PrintDebugInfo ();

        global_refl->Flush (frameIndex);
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