#include "Assert.hpp"
#include "Camera.hpp"
#include "FullscreenQuad.hpp"
#include "GLFWWindow.hpp"
#include "GraphRenderer.hpp"
#include "GraphSettings.hpp"
#include "Logger.hpp"
#include "Noncopyable.hpp"
#include "Operation.hpp"
#include "RenderGraph.hpp"
#include "Resource.hpp"
#include "SDLWindow.hpp"
#include "Time.hpp"
#include "Timer.hpp"
#include "UniformBlock.hpp"
#include "Utils.hpp"
#include "VulkanEnvironment.hpp"

// from VulkanWrapper
#include "VulkanWrapper.hpp"

#include "DeviceExtra.hpp"
#include "ShaderReflection.hpp"

#include "CameraControl.hpp"

#include <array>
#include <cstdint>
#include <iostream>
#include <iterator>
#include <memory>
#include <optional>
#include <set>
#include <vector>

#include "glmlib.hpp"

#include <atomic>
#include <thread>

#include <vulkan/vulkan.h>


int main (int, char**)
{
    Window::U window = GLFWWindow::Create ();

    KeyboardState keyboard;
    MouseState    mouse;

    window->events.focused += [] () {
        std::cout << "window focused" << std::endl;
    };

    window->events.resized += [] (uint32_t width, uint32_t height) {
        std::cout << "window resized to " << width << " x " << height << std::endl;
    };

    window->events.leftMouseButtonPressed += [&] (auto...) {
        mouse.leftButton = true;
    };
    window->events.leftMouseButtonReleased += [&] (auto...) {
        mouse.leftButton = false;
    };


    VulkanEnvironment::U testenv = VulkanEnvironment::CreateForBuildType (*window);

    Device&      device        = *testenv->device;
    CommandPool& commandPool   = *testenv->commandPool;
    Queue&       graphicsQueue = *testenv->graphicsQueue;
    Swapchain&   swapchain     = *testenv->swapchain;

    ImageTransferable::U img = ImageTransferable::Create (device, graphicsQueue, commandPool, ImageBase::RGBA, 512, 512, 0);

    std::vector<uint8_t> pix (512 * 512 * 4, 127);
    img->CopyTransitionTransfer (ImageBase::INITIAL_LAYOUT, pix.data (), pix.size ());


    DeviceExtra d {device, commandPool, graphicsQueue};


    Camera c (glm::vec3 (0, 0, 0.5), glm::vec3 (0, 0, -1), window->GetAspectRatio ());

    CameraControl::Settings cameraControlSettings {
        window->events.keyPressed,
        window->events.keyReleased,
        window->events.leftMouseButtonPressed,
        window->events.leftMouseButtonReleased,
        window->events.mouseMove,
    };

    CameraControl cameraControl (c, cameraControlSettings);


    using namespace RG;

    RenderGraph graph (device, commandPool);

    class : public ShaderSourceBuilder {
    public:
        virtual std::string GetProvidedShaderSource () const override
        {
            return R"(#version 450
#extension GL_ARB_separate_shader_objects : enable
)";
        }
    } versionBuilder;


    UniformBlock Time (0, "Time", "time", {
                                              {"time", ST::vec1},
                                              {"VP", ST::mat4},
                                          });

    FullscreenQuad::P fq = FullscreenQuad::CreateShared (device, graphicsQueue, commandPool);

    std::vector<const ShaderSourceBuilder*> builders;
    builders.push_back (&versionBuilder);
    builders.push_back (&Time);
    for (auto& a : fq->GetShaderBuilders ()) {
        builders.push_back (a);
    }


    ShaderPipeline::P sp = ShaderPipeline::CreateShared (device);
    sp->SetVertexShader (R"(
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (std140, binding = 0) uniform Time {
    float time;
    mat4 VP;
} time;

layout (location = 0) in vec2 position;
layout (location = 1) in vec2 uv;

layout (location = 0) out vec2 textureCoords;

void main ()
{
    gl_Position =  time.VP * vec4 (position + vec2 (0 / 100.f), 0.0, 1.0);
    textureCoords = uv;
}
    )");

    sp->SetFragmentShader (R"(
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (binding = 1) uniform sampler2D agySampler;

layout (location = 0) in vec2 uv;

layout (location = 2) out vec4 presented;
layout (location = 0) out vec4 copy[2];

void main ()
{
    vec4 result = vec4 (vec3 (uv, 1.f), 1);
    presented = vec4 (texture (agySampler, uv).rgb, 1);
    //presented = result;
    copy[0] = vec4 (texture (agySampler, uv).rgb, 1);
    copy[1] = result;
}
    )");

    auto asddd = fq->GetShaderBuilders ();


    std::string s2354 = Time.GetProvidedShaderSource ();
    std::string s2355 = asddd[0]->GetProvidedShaderSource ();

    struct {
        glm::mat4& VP;
        float&     time;
    } TimeV {
        Time.GetRef<glm::mat4> ("VP"),
        Time.GetRef<float> ("time"),
    };


    SwapchainImageResource& presented = graph.CreateResourceTyped<SwapchainImageResource> (swapchain);

    ReadOnlyImageResource& agy = graph.CreateResourceTyped<ReadOnlyImageResource> (SingleImageResource::Format, 512, 512);

    ImageResource&        presentedCopy = graph.CreateResourceTyped<ImageResource> (2);
    UniformBlockResource& unif          = graph.CreateResourceTyped<UniformBlockResource> (Time.GetSize ());

    Operation& redFillOperation = graph.CreateOperationTyped<RenderOperation> (fq, sp);

    GraphSettings s (device, graphicsQueue, commandPool, swapchain);
    graph.CompileResources (s);

    agy.image->CopyTransitionTransfer (VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, pix.data (), pix.size (), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    graph.AddConnection (RenderGraph::InputConnection {redFillOperation, 0, unif});
    graph.AddConnection (RenderGraph::InputConnection {redFillOperation, 1, agy});
    graph.AddConnection (RenderGraph::OutputConnection {redFillOperation, 0, presentedCopy});
    graph.AddConnection (RenderGraph::OutputConnection {redFillOperation, 2, presented});

    graph.Compile (s);

    TimeV.VP = glm::mat4 (1.f);


    SynchronizedSwapchainGraphRenderer renderer (graph, swapchain);

    renderer.preSubmitEvent += [&] (uint32_t frameIndex, uint64_t deltaNs) {
        TimePoint delta (deltaNs);

        const float dt = delta.AsSeconds ();

        cameraControl.UpdatePosition (dt);

        TimeV.VP   = c.GetViewProjectionMatrix ();
        TimeV.time = TimePoint::SinceApplicationStart ().AsSeconds ();

        unif.GetMapping (frameIndex).Copy (Time.GetData (), Time.GetSize (), 0);
    };

    bool quit = false;

    constexpr uint8_t ESC_CODE = 27;

    window->events.keyPressed += [&] (uint32_t a) {
        if (a == ESC_CODE) {
            // window->ToggleFullscreen ();
            quit = false;
        }
        return;
    };

    window->DoEventLoop (renderer.GetInfiniteDrawCallback ([&] { return quit; }));

    vkQueueWaitIdle (graphicsQueue);
    vkDeviceWaitIdle (device);
}
