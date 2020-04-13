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
#include "Utils.hpp"
#include "tests/VulkanTestEnvironment.hpp"

// from VulkanWrapper
#include "VulkanWrapper.hpp"

#include "ShaderReflection.hpp"

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
class CallEvery {
private:
    std::atomic<bool>      stop;
    std::function<void ()> callback;
    std::thread            timerThread;
    const int32_t          waitNs;
    uint32_t               lastDiff;

    void ThreadFunc ()
    {
        callback ();

        // auto start = std::chrono::high_resolution_clock::now ();

        std::this_thread::sleep_for (std::chrono::nanoseconds (waitNs));

        // auto                                     end     = std::chrono::high_resolution_clock::now ();
        // std::chrono::duration<double, std::nano> elapsed = end - start;
        // const int32_t waitedNs   = elapsed.count ();
        // const int32_t waitedDiff = std::abs (waitedNs - waitNs);
        // std::cout << "Waited " << waitedNs << " ns instead of " << waitNs << " ns (diff is " << ((elapsed.count () - waitNs) * 1e6) << " ms)" << std::endl;
        // std::cout << "percent: " << static_cast<double> (waitNs + waitedDiff) / waitNs << " more" << std::endl;

        if (!stop) {
            ThreadFunc ();
        }
    }

public:
    CallEvery (double waitInSeconds, std::function<void ()> callback)
        : callback (callback)
        , waitNs (waitInSeconds * 1e9)
        , lastDiff (0)
        , timerThread (std::bind (&CallEvery::ThreadFunc, this))
        , stop (false)
    {
    }

    ~CallEvery ()
    {
        stop = true;
        timerThread.join ();
    }
};

int main (int argc, char* argv[])
{
    Window::U window = GLFWWindow::Create ();

    window->events.focused += [] () {
        std::cout << "window focused" << std::endl;
    };

    window->events.resized += [] (uint32_t width, uint32_t height) {
        std::cout << "window resized to " << width << " x " << height << std::endl;
    };

    TestEnvironment testenv ({VK_EXT_DEBUG_UTILS_EXTENSION_NAME}, *window);

    Device&      device        = *testenv.device;
    CommandPool& commandPool   = *testenv.commandPool;
    Queue&       graphicsQueue = *testenv.graphicsQueue;
    Swapchain&   swapchain     = *testenv.swapchain;

    DeviceExtra d {device, commandPool, graphicsQueue};

    using namespace RenderGraphns;

    Camera c (glm::vec3 (0, 0, 0.5), glm::vec3 (0, 0, -1), window->GetAspectRatio ());

    constexpr uint32_t             MAX_KEYCOUNT = 1024;
    std::array<bool, MAX_KEYCOUNT> pressedKeys;
    pressedKeys.fill (false);

    window->events.keyPressed += [&] (uint32_t keyCode) {
        if (ASSERT (keyCode < MAX_KEYCOUNT)) {
            pressedKeys[keyCode] = true;
        }
    };

    window->events.keyReleased += [&] (uint32_t keyCode) {
        if (ASSERT (keyCode < MAX_KEYCOUNT)) {
            pressedKeys[keyCode] = false;
        }
    };

    c.positionChanged += [] (glm::vec3 pos) {
        // TODO why is this called every time
        //std::cout << "camera moved" << std::endl;
    };

    bool mouseDown = false;

    window->events.leftMouseButtonPressed += [&] (auto...) {
        mouseDown = true;
    };
    window->events.leftMouseButtonReleased += [&] (auto...) {
        mouseDown = false;
    };

    window->events.mouseMove += [&] (uint32_t x, uint32_t y) {
        static glm::vec2 lastPos (x, y);
        glm::vec2        currentPos (x, y);
        if (mouseDown) {
            c.ProcessMouseInput (lastPos - currentPos);
        }
        lastPos = currentPos;
    };


    RenderGraph graph (device, commandPool);

    FullscreenQuad::P fq = FullscreenQuad::CreateShared (device, graphicsQueue, commandPool);

    ShaderPipeline::P sp = ShaderPipeline::CreateShared (device);
    sp->SetVertexShader (R"(
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (binding = 0) uniform Time {
    mat4 VP;
    float time;
} time;

layout (location = 0) in vec2 position;
layout (location = 1) in vec2 uv;

layout (location = 0) out vec2 textureCoords;
layout (location = 1) out float asdout;


void main ()
{
    gl_Position =  time.VP * vec4 (position + vec2 (0 / 100.f), 0.0, 1.0);
    textureCoords = uv;
}
    )");

    sp->SetFragmentShader (R"(
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (binding = 0) uniform Time {
    mat4 VP;
    float time;
} time;

layout (location = 0) in vec2 uv;

layout (location = 2) out vec4 presented;
layout (location = 0) out vec4 copy[2];

void main ()
{
    vec4 result = vec4 (vec3 (uv, 1.f), 1);
    presented = result;
    copy[0] = result;
    copy[1] = result;
}
    )");

    SwapchainImageResource& presented     = graph.CreateResourceTyped<SwapchainImageResource> (swapchain);
    ImageResource&          presentedCopy = graph.CreateResourceTyped<ImageResource> (2);
    UniformBlockResource&   unif          = graph.CreateResourceTyped<UniformBlockResource> (16 * sizeof (float) + sizeof (float));

    //struct UniformResourceBinding {
    //    std::map<std::string, UniformBlockResource::Ref> uniformNameMapping;

    //    void Register (std::string name, UniformBlockResource::U& uniformBlock)
    //    {
    //        uniformNameMapping[name] = *uniformBlock;
    //    }
    //};

    Operation& redFillOperation = graph.CreateOperationTyped<RenderOperation> (fq, sp);

    GraphSettings s (device, graphicsQueue, commandPool, swapchain);
    graph.CompileResources (s);

    graph.AddConnection (RenderGraph::InputConnection {redFillOperation, 0, unif});
    graph.AddConnection (RenderGraph::OutputConnection {redFillOperation, 0, presentedCopy});
    graph.AddConnection (RenderGraph::OutputConnection {redFillOperation, 2, presented});

    graph.Compile (s);

    struct TimeUniform {
        glm::mat4 VP;
        float     time;
    };

    SynchronizedSwapchainGraphRenderer swapchainSync (graph, swapchain, [&] (uint32_t frameIndex) {
        TimePoint currentTime = TimePoint::SinceApplicationStart ();

        constexpr float dt = 1 / 60.f;
        if (pressedKeys['W']) {
            c.Move (Camera::MovementDirection::Forward, dt);
        }
        if (pressedKeys['A']) {
            c.Move (Camera::MovementDirection::Left, dt);
        }
        if (pressedKeys['S']) {
            c.Move (Camera::MovementDirection::Backward, dt);
        }
        if (pressedKeys['D']) {
            c.Move (Camera::MovementDirection::Right, dt);
        }
        if (pressedKeys['E']) {
            c.Move (Camera::MovementDirection::Down, dt);
        }
        if (pressedKeys['Q']) {
            c.Move (Camera::MovementDirection::Up, dt);
        }

        TimeUniform t;
        t.VP = c.GetViewProjectionMatrix ();
        //t.VP = glm::mat4 (1.0);
        //t.VP = frustum.GetMatrix () * v;

        t.time = currentTime.AsSeconds ();

        unif.GetMapping (frameIndex).Copy (t);
    });


    bool quit = false;

    constexpr uint8_t ESC_CODE = 27;

    auto a = Event<uint32_t>::CreateCallback ([&] (uint32_t a) {
        if (a == ESC_CODE) {
            window->ToggleFullscreen ();
            //quit = false;
        }
        return;
    });

    window->events.keyPressed += a;

    window->DoEventLoop (swapchainSync.GetInfiniteDrawCallback ([&] { return quit; }));

    vkQueueWaitIdle (graphicsQueue);
    vkDeviceWaitIdle (device);
}
