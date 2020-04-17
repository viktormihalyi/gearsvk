#include "GearsAPIv2.hpp"

#include "GLFWWindow.hpp"
#include "GraphRenderer.hpp"
#include "RenderGraph.hpp"
#include "Tests/VulkanTestEnvironment.hpp"
#include "core/Sequence.h"

#include <atomic>
#include <memory>
#include <thread>

using namespace RenderGraphns;


Window::U          window;
TestEnvironment::U env;
RenderGraph::U     renderGraph;


#define PRECOND_THROW(cond)                               \
    if (ERROR (!(cond))) {                                \
        throw std::runtime_error ("precondition failed"); \
    }                                                     \
    (void)0


void InitializeEnvironment ()
{
    window = HiddenGLFWWindow::Create (); // create a hidden window by default
    env    = TestEnvironment::Create (std::vector<const char*> {VK_EXT_DEBUG_UTILS_EXTENSION_NAME}, *window);
}


void DestroyEnvironment ()
{
    env->Wait ();
    env.reset ();
    window.reset ();
}


void SetRenderGraphFromSequence (Sequence::P seq)
{
    PRECOND_THROW (env != nullptr);

    renderGraph = RenderGraph::Create (*env->device, *env->commandPool);

    auto sp = ShaderPipeline::Create (*env->device);
    sp->SetVertexShader (R"(
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

    sp->SetFragmentShader (R"(
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec2 uv;

layout (location = 0) out vec4 outColor;
layout (location = 1) out vec4 outCopy;

void main () {
    vec4 result = vec4 (uv, 0, 1);
    outColor = result;
    outCopy = result;
}
    )");

    GraphSettings s (*env->device, *env->graphicsQueue, *env->commandPool, *env->swapchain);

    Resource& presentedCopy = renderGraph->AddResource (ImageResource::Create ());
    Resource& presented     = renderGraph->AddResource (SwapchainImageResource::Create (*env->swapchain));

    Operation& redFillOperation = renderGraph->AddOperation (RenderOperation::Create (DrawRecordableInfo::CreateShared (1, 6),
                                                                                      std::move (sp)));

    renderGraph->CompileResources (s);

    renderGraph->AddConnection (RenderGraph::OutputConnection {redFillOperation, 0, presented});
    renderGraph->AddConnection (RenderGraph::OutputConnection {redFillOperation, 1, presentedCopy});

    renderGraph->Compile (s);
}


void StartRendering (const std::function<bool ()>& doRender)
{
    PRECOND_THROW (env != nullptr);
    PRECOND_THROW (renderGraph != nullptr);
    env->Wait ();

    SynchronizedSwapchainGraphRenderer swapchainSync (*renderGraph, *env->swapchain, [] (uint32_t) {});

    window->Show ();

    window->DoEventLoop (swapchainSync.GetInfiniteDrawCallback (doRender));

    window->Hide ();
    window.reset ();

    env->Wait ();

    {
        Utils::TimerLogger tl ("switching to new window");
        Utils::TimerScope  ts (tl);

        window = HiddenGLFWWindow::Create ();

        env->physicalDevice->RecreateForSurface (window->GetSurface (*env->instance));

        env->swapchain->RecreateForSurface (window->GetSurface (*env->instance));

        GraphSettings s (*env->device, *env->graphicsQueue, *env->commandPool, *env->swapchain);

        renderGraph->CompileResources (s);
        renderGraph->Compile (s);
    }
}
