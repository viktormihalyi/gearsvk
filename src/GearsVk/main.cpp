#include "Assert.hpp"
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

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vulkan/vulkan.h>


int main (int argc, char* argv[])
{
    Window::U window = SDLWindow::Create ();

    window->events.focused.Attach ([] () {
        std::cout << "window focused" << std::endl;
    });
    //h.Detach ();

    constexpr uint8_t ESC_CODE = 27;
    TestEnvironment   testenv ({VK_EXT_DEBUG_UTILS_EXTENSION_NAME}, *window);

    Device&      device        = *testenv.device;
    CommandPool& commandPool   = *testenv.commandPool;
    Queue&       graphicsQueue = *testenv.graphicsQueue;
    Swapchain&   swapchain     = *testenv.swapchain;

    DeviceExtra d {device, commandPool, graphicsQueue};

    using namespace RenderGraphns;

    RenderGraph graph (device, commandPool, GraphSettings (device, graphicsQueue, commandPool, swapchain));

    FullscreenQuad::P fq = FullscreenQuad::CreateShared (device, graphicsQueue, commandPool);

    ShaderPipeline::P sp = ShaderPipeline::CreateShared (device);
    sp->SetVertexShader (R"(
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (binding = 0) uniform Time {
    float time;
} time;

layout (location = 0) in vec2 position;
layout (location = 1) in vec2 uv;
layout (location = 2) in float asd;

layout (location = 0) out vec2 textureCoords;
layout (location = 1) out float asdout;


void main ()
{
    gl_Position = vec4 (position + vec2 (time.time / 100.f), 0.0, 1.0);
    textureCoords = uv;
    asdout = asd;
}
    )");

    sp->SetFragmentShader (R"(
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (binding = 0) uniform Time {
    float time;
} time;

layout (location = 1) in float asdout;
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
    UniformBlockResource&   unif          = graph.CreateResourceTyped<UniformBlockResource> (4);

    struct UniformResourceBinding {
        std::map<std::string, UniformBlockResource::Ref> uniformNameMapping;

        void Register (std::string name, UniformBlockResource::U& uniformBlock)
        {
            uniformNameMapping[name] = *uniformBlock;
        }
    };

    Operation& redFillOperation = graph.CreateOperationTyped<RenderOperation> (fq, sp);

    graph.AddConnection (RenderGraph::InputConnection {redFillOperation, 0, unif});
    graph.AddConnection (RenderGraph::OutputConnection {redFillOperation, 0, presentedCopy});
    graph.AddConnection (RenderGraph::OutputConnection {redFillOperation, 2, presented});

    graph.Compile ();

    BlockingGraphRenderer swapchainSync (graph, swapchain, [&] (uint32_t frameIndex) {
        Time::Point currentTime  = Time::SinceApplicationStart ();
        float       timeInSconds = currentTime.AsSeconds ();
        unif.GetMapping (frameIndex).Copy (timeInSconds);
    });


    bool quit = false;

    auto a = Event<uint32_t>::CreateCallback ([&] (uint32_t a) {
        std::cout << a << std::endl;
        return;
    });

    window->events.keyPressed += a;
    window->events.keyPressed -= a;

    window->DoEventLoop (swapchainSync.GetInfiniteDrawCallback ([&] { return quit; }));
}
