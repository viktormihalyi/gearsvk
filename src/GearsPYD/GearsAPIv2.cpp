#include "GearsAPIv2.hpp"

#include "FullscreenQuad.hpp"
#include "GLFWWindow.hpp"
#include "GraphRenderer.hpp"
#include "Persistent.hpp"
#include "RenderGraph.hpp"
#include "VulkanEnvironment.hpp"
#include "core/Sequence.h"

#include <atomic>
#include <memory>
#include <set>
#include <thread>
#include <type_traits>

using namespace RG;

Window::U            window;
VulkanEnvironment::U env;
RenderGraph::U       renderGraph;

PersistentString asd ("asd");

struct ASDD {
    int a;
    int a2;
};

Persistent<ASDD>      asd2 ("asd2");
Persistent<int>       asd3 ("asd3");
Persistent<glm::vec4> asd4 ("asd4");


void InitializeEnvironment ()
{
    window = HiddenGLFWWindow::Create (); // create a hidden window by default
    env    = VulkanEnvironment::Create (*window);
}


void DestroyEnvironment ()
{
    env.reset ();
    window.reset ();
}


void SetRenderGraphFromSequence (Sequence::P seq)
{
    ASSERT_THROW (env != nullptr);

    Stimulus::CP stim      = seq->getStimulusAtFrame (60);
    auto         passes    = stim->getPasses ();
    auto         firstPass = passes[0];
    auto         vert      = firstPass->getStimulusGeneratorVertexShaderSource (Pass::RasterizationMode::fullscreen);
    auto         frag      = firstPass->getStimulusGeneratorShaderSource ();

    auto seqpip = ShaderPipeline::CreateShared (*env->device);
    seqpip->SetVertexShaderFromString (vert);
    seqpip->SetFragmentShaderFromString (frag);

    renderGraph = RenderGraph::Create (*env->device, *env->commandPool);


    GraphSettings s (*env->device, *env->graphicsQueue, *env->commandPool, *env->swapchain);

    SwapchainImageResource& presented = renderGraph->CreateResource<SwapchainImageResource> (*env->swapchain);
    Resource&               unif      = renderGraph->CreateResource<UniformBlockResource> (8);
    CPUBufferResource&      cpubuffer = renderGraph->CreateResource<CPUBufferResource> (1024);

    Operation& redFillOperation = renderGraph->AddOperation (RenderOperation::Create (DrawRecordableInfo::CreateShared (1, 6), seqpip));

    renderGraph->CompileResources (s);

    renderGraph->CreateOutputConnection (redFillOperation, 0, presented);
    renderGraph->CreateInputConnection<UniformInputBinding> (redFillOperation, 0, cpubuffer);

    renderGraph->Compile (s);
}


void StartRendering (const std::function<bool ()>& doRender)
{
    ASSERT_THROW (env != nullptr);
    ASSERT_THROW (renderGraph != nullptr);

    env->Wait ();

    SynchronizedSwapchainGraphRenderer swapchainSync (*renderGraph, *env->swapchain);

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

        GraphSettings s (*env->device, *env->graphicsQueue, *env->commandPool, *env->swapchain);

        renderGraph->CompileResources (s);
        renderGraph->Compile (s);
    }
}

void TryCompile (ShaderModule::ShaderKind shaderKind, const std::string& source)
{
    try {
        ShaderModule::CreateFromGLSLString (*env->device, shaderKind, source);
        std::cout << "compile succeeded" << std::endl;
    } catch (const ShaderCompileException&) {
        std::cout << "compile failed, source code: " << std::endl;
        std::cout << source << std::endl;
    }
}