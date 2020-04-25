#include "Assert.hpp"
#include "Camera.hpp"
#include "FullscreenQuad.hpp"
#include "GLFWWindow.hpp"
#include "GraphRenderer.hpp"
#include "GraphSettings.hpp"
#include "Logger.hpp"
#include "MultithreadedFunction.hpp"
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

    VulkanEnvironment::U testenv = VulkanEnvironment::CreateForBuildType (*window);

    Device&      device        = *testenv->device;
    CommandPool& commandPool   = *testenv->commandPool;
    Queue&       graphicsQueue = *testenv->graphicsQueue;
    Swapchain&   swapchain     = *testenv->swapchain;

    DeviceExtra d {device, commandPool, graphicsQueue};


    Camera c (glm::vec3 (0, 0, 0.5), glm::vec3 (0, 0, -1), window->GetAspectRatio ());

    CameraControl cameraControl (c, window->events);


    using namespace RG;

    const GraphSettings s (device, graphicsQueue, commandPool, swapchain);
    RenderGraph         graph (device, commandPool);

    const ShaderStruct TimeType ({
        {"time", ST::vec1},
    });
    const ShaderStruct CameraStruct ({
        {"VP", ST::mat4},
        {"rayDirMatrix", ST::mat4},
        {"position", ST::vec3},
    });

    UniformBlock Time (0, "time", TimeType);
    UniformBlock CameraUniform (1, "camera", CameraStruct);

    float&     time        = Time.GetRef<float> ("time");
    glm::mat4& VP          = CameraUniform.GetRef<glm::mat4> ("VP");
    glm::mat4& rayDir      = CameraUniform.GetRef<glm::mat4> ("rayDirMatrix");
    glm::vec3& camPosition = CameraUniform.GetRef<glm::vec3> ("position");


    FullscreenQuad::P fq = FullscreenQuad::CreateShared (device, graphicsQueue, commandPool);

    ShaderPipeline::P sp = ShaderPipeline::CreateShared (device);
    sp->SetVertexShader (R"(
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (std140, binding = 0) uniform Time {
    float time;
} time;

layout (std140, binding = 3) uniform Camera {
    mat4 VP;
    mat4 rayDirMatrix;
    vec3 position;
} camera;

layout (location = 0) in vec2 position;
layout (location = 1) in vec2 uv;

layout (location = 0) out vec2 textureCoords;
layout (location = 1) out vec3 rayDirection;

void main ()
{
    gl_Position =  camera.VP * vec4 (position + vec2 (0 / 100.f), 0.0, 1.0);
    textureCoords = uv;
    rayDirection = (camera.rayDirMatrix * vec4 (position, 0, 1)).xyz;
}
    )");

    sp->SetFragmentShader (R"(
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (std140, binding = 0) uniform Time {
    float time;
} time;

layout (std140, binding = 3) uniform Camera {
    mat4 VP;
    mat4 rayDirMatrix;
    vec3 position;
} camera;

layout (binding = 1) uniform sampler2D agy2dSampler;
layout (binding = 2) uniform sampler3D agySampler;

layout (location = 0) in vec2 uv;
layout (location = 1) in vec3 rayDirection;

layout (location = 2) out vec4 presented;
layout (location = 0) out vec4 copy[2];

void main ()
{
    vec4 result = vec4 (vec3 (uv, 1.f), 1);
    presented = vec4 (texture (agySampler, vec3(uv, mod (time.time, fract (time.time*0.1f))  )).rrr, 1);
    //presented = result;
    //presented = vec4 (normalize (rayDirection) * 0.5 + 0.5, 1);

    vec3 normRayDir = normalize (rayDirection);
    vec3 rayStart = camera.position;
    float rayT = 0.f;
    float rayStep = 0.02f;
    float val = 0.f;

    for (int i = 0; i < 64; ++i) {
        vec3 rayPos = rayStart + rayT * rayStep;
        val += texture (agySampler, rayPos).r;
        rayT += rayStep;
    }

    //presented = vec4 (vec3 (val), 1);

    copy[0] = vec4 (texture (agy2dSampler, uv).rgb, 1);
    copy[1] = result;
}
    )");

    // resources
    SwapchainImageResource& presented        = graph.CreateResource<SwapchainImageResource> (swapchain);
    ReadOnlyImageResource&  agy              = graph.CreateResource<ReadOnlyImageResource> (VK_FORMAT_R8G8B8A8_SRGB, 512, 512);
    ReadOnlyImageResource&  agy3d            = graph.CreateResource<ReadOnlyImageResource> (VK_FORMAT_R8_SRGB, 256, 256, 256);
    WritableImageResource&  presentedCopy    = graph.CreateResource<WritableImageResource> (2);
    UniformBlockResource&   unif             = graph.CreateResource<UniformBlockResource> (TimeType);
    UniformBlockResource&   cameraUniformRes = graph.CreateResource<UniformBlockResource> (CameraStruct);


    // operations
    Operation& redFillOperation = graph.CreateOperation<RenderOperation> (fq, sp);


    graph.CompileResources (s);

    Image2DTransferable::U img = Image2DTransferable::Create (device, graphicsQueue, commandPool, ImageBase::RGBA, 512, 512, 0);
    std::vector<uint8_t>   pix (512 * 512 * 4, 127);
    img->CopyTransitionTransfer (ImageBase::INITIAL_LAYOUT, pix.data (), pix.size ());

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


    {
        Utils::TimerLogger    l ("transforming volume data");
        Utils::TimerScope     s (l);
        MultithreadedFunction d ([&] (uint32_t threadCount, uint32_t threadIndex) {
            const uint32_t pixelCount = 4096 * 4096;
            for (uint32_t bidx = pixelCount / threadCount * threadIndex; bidx < pixelCount / threadCount * (threadIndex + 1); ++bidx) {
                sliceData2[BrainDataIndexMapping (bidx)] = brainData[bidx];
            }
        });
    }


    agy3d.CopyTransitionTransfer (sliceData2);


    graph.AddConnection (RenderGraph::InputConnection {redFillOperation, 0, unif});
    graph.AddConnection (RenderGraph::InputConnection {redFillOperation, 3, cameraUniformRes});
    graph.AddConnection (RenderGraph::InputConnection {redFillOperation, 1, agy});
    graph.AddConnection (RenderGraph::InputConnection {redFillOperation, 2, agy3d});
    graph.AddConnection (RenderGraph::OutputConnection {redFillOperation, 0, presentedCopy});
    graph.AddConnection (RenderGraph::OutputConnection {redFillOperation, 2, presented});

    graph.Compile (s);

    SynchronizedSwapchainGraphRenderer renderer (graph, swapchain);

    //renderer.recreateEvent += [&] () {
    //    agy3d.CopyTransitionTransfer (sliceData2);
    //};

    VP = glm::mat4 (1.f);

    renderer.preSubmitEvent += [&] (uint32_t frameIndex, uint64_t deltaNs) {
        TimePoint delta (deltaNs);

        const float dt = delta.AsSeconds ();

        cameraControl.UpdatePosition (dt);

        VP          = c.GetViewProjectionMatrix ();
        rayDir      = c.GetRayDirMatrix ();
        time        = TimePoint::SinceApplicationStart ().AsSeconds ();
        camPosition = c.GetPosition ();

        unif.Set (frameIndex, Time);
        cameraUniformRes.Set (frameIndex, CameraUniform);
    };

    bool quit = false;

    constexpr uint8_t ESC_CODE = 27;

    window->events.keyPressed += [&] (uint32_t a) {
        if (a == ESC_CODE) {
            // window->ToggleFullscreen ();
            quit = true;
        }
        return;
    };

    window->DoEventLoop (renderer.GetInfiniteDrawCallback ([&] { return quit; }));

    vkQueueWaitIdle (graphicsQueue);
    vkDeviceWaitIdle (device);
}
