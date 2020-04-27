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
    DeviceExtra& deviceExtra   = *testenv->deviceExtra;


    Camera c (glm::vec3 (-1, 0, 0.5f), glm::vec3 (1, 0.2, 0), window->GetAspectRatio ());

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

    FullscreenQuad::P fq = FullscreenQuad::CreateShared (deviceExtra);

    ShaderPipeline::P sp = ShaderPipeline::CreateShared (device);
    sp->AddShaders ({
        PROJECT_ROOT / "shaders" / "brain.vert",
        PROJECT_ROOT / "shaders" / "brain.frag",
    });

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

        //CameraUniform["VP"] = c.GetViewProjectionMatrix ();
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
