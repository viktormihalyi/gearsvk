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

    Image2DTransferable::U img = Image2DTransferable::Create (device, graphicsQueue, commandPool, ImageBase::RGBA, 512, 512, 0);

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

    ShaderStruct TimeType ({
        {"time", ST::vec1},
        {"VP", ST::mat4},
        {"rayDir", ST::mat4},
    });

    UniformBlock Time (0, "time", TimeType);

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
    mat4 rayDir;
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

layout (std140, binding = 0) uniform Time {
    float time;
    mat4 VP;
    mat4 rayDir;
} time;

layout (binding = 1) uniform sampler2D agy2dSampler;
layout (binding = 2) uniform sampler3D agySampler;

layout (location = 0) in vec2 uv;

layout (location = 2) out vec4 presented;
layout (location = 0) out vec4 copy[2];

void main ()
{
    vec4 result = vec4 (vec3 (uv, 1.f), 1);
    presented = vec4 (texture (agySampler, vec3(uv, mod (time.time, fract (time.time*0.1f))  )).rrr, 1);
    //presented = result;
    copy[0] = vec4 (texture (agy2dSampler, uv).rgb, 1);
    copy[1] = result;
}
    )");

    auto asddd = fq->GetShaderBuilders ();


    std::string s2354 = Time.GetProvidedShaderSource ();
    std::string s2355 = asddd[0]->GetProvidedShaderSource ();

    struct {
        glm::mat4& VP;
        float&     time;
        glm::mat4& rayDir;
    } TimeV {
        Time.GetRef<glm::mat4> ("VP"),
        Time.GetRef<float> ("time"),
        Time.GetRef<glm::mat4> ("rayDir"),
    };


    SwapchainImageResource& presented = graph.CreateResource<SwapchainImageResource> (swapchain);

    ReadOnlyImageResource& agy   = graph.CreateResource<ReadOnlyImageResource> (SingleImageResource::FormatRGBA, 512, 512);
    ReadOnlyImageResource& agy3d = graph.CreateResource<ReadOnlyImageResource> (VK_FORMAT_R8_SRGB, 4096 / 16, 4096 / 16, 16 * 16);

    WritableImageResource& presentedCopy = graph.CreateResource<WritableImageResource> (2);
    UniformBlockResource&  unif          = graph.CreateResource<UniformBlockResource> (TimeType);

    Operation& redFillOperation = graph.CreateOperation<RenderOperation> (fq, sp);

    GraphSettings s (device, graphicsQueue, commandPool, swapchain);
    graph.CompileResources (s);

    agy.CopyTransitionTransfer (pix);

    std::vector<uint8_t> brainData = ReadImage (PROJECT_ROOT / "brain.jpg", 1);

    std::vector<uint8_t> sliceData2 (256 * 256 * 256);

    auto constexpr BrainDataIndexMapping = [] (uint32_t oirignalDataIndex) {
        const uint32_t x = oirignalDataIndex % 4096;
        const uint32_t y = oirignalDataIndex / 4096;

        const uint32_t row        = y % 256;
        const uint32_t column     = x % 256;
        const uint32_t sliceIndex = 16 * (y / 256) + (x / 256);

        return sliceIndex * 256 * 256 + row + column * 256;
    };

    for (uint32_t bidx = 0; bidx < 4096 * 4096; ++bidx) {
        sliceData2[BrainDataIndexMapping (bidx)] = brainData[bidx];
    }


    agy3d.CopyTransitionTransfer (sliceData2);


    graph.AddConnection (RenderGraph::InputConnection {redFillOperation, 0, unif});
    graph.AddConnection (RenderGraph::InputConnection {redFillOperation, 1, agy});
    graph.AddConnection (RenderGraph::InputConnection {redFillOperation, 2, agy3d});
    graph.AddConnection (RenderGraph::OutputConnection {redFillOperation, 0, presentedCopy});
    graph.AddConnection (RenderGraph::OutputConnection {redFillOperation, 2, presented});

    graph.Compile (s);

    SynchronizedSwapchainGraphRenderer renderer (graph, swapchain);

    //renderer.recreateEvent += [&] () {
    //    agy3d.CopyTransitionTransfer (sliceData2);
    //};

    renderer.preSubmitEvent += [&] (uint32_t frameIndex, uint64_t deltaNs) {
        TimePoint delta (deltaNs);

        const float dt = delta.AsSeconds ();

        cameraControl.UpdatePosition (dt);

        TimeV.VP     = c.GetViewProjectionMatrix ();
        TimeV.rayDir = c.GetRayDirMatrix ();
        TimeV.time   = TimePoint::SinceApplicationStart ().AsSeconds ();

        unif.Set (frameIndex, Time);
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
