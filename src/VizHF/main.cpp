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


const std::filesystem::path ShadersFolder = PROJECT_ROOT / "src" / "VizHF" / "shaders";


int main (int, char**)
{
    Window::U window = GLFWWindow::Create ();

    VulkanEnvironment::U testenv = VulkanEnvironment::Create (*window);

    Device&      device        = *testenv->device;
    CommandPool& commandPool   = *testenv->commandPool;
    Queue&       graphicsQueue = *testenv->graphicsQueue;
    Swapchain&   swapchain     = *testenv->swapchain;
    DeviceExtra& deviceExtra   = *testenv->deviceExtra;

    Camera        c (glm::vec3 (-1, 0, 0.5f), glm::vec3 (1, 0.0f, 0), window->GetAspectRatio ());
    CameraControl cameraControl (c, window->events);

    const RG::GraphSettings s (device, graphicsQueue, commandPool, swapchain);
    RG::RenderGraph         graph (device, commandPool);


    // ========================= GRAPH OPERATIONS =========================

    ShaderPipeline::P sp = ShaderPipeline::CreateShared (device);
    sp->SetShadersFromSourceFiles ({
        ShadersFolder / "brain.vert",
        ShadersFolder / "brain.frag",
    });

    RG::RenderOperation& brainRenderOp = graph.CreateOperation<RG::RenderOperation> (FullscreenQuad::CreateShared (deviceExtra), sp);


    // ========================= GRAPH RESOURCES =========================

    RG::SwapchainImageResource&    presented = graph.CreateResource<RG::SwapchainImageResource> (swapchain);
    RG::ReadOnlyImageResource&     matcap    = graph.CreateResource<RG::ReadOnlyImageResource> (VK_FORMAT_R8G8B8A8_SRGB, 512, 512);
    RG::ReadOnlyImageResource&     agy3d     = graph.CreateResource<RG::ReadOnlyImageResource> (VK_FORMAT_R8_SRGB, 256, 256, 256);
    RG::UniformReflectionResource& refl      = graph.CreateResource<RG::UniformReflectionResource> (sp, RG::UniformReflectionResource::Strategy::UniformBlocksOnly);


    // ========================= GRAPH CONNECTIONS =========================

    for (uint32_t i = 0; i < refl.uboRes.size (); ++i) {
        graph.CreateInputConnection<RG::UniformInputBinding> (brainRenderOp, refl.bindings[i], *refl.uboRes[i]);
    }
    graph.CreateInputConnection<RG::ImageInputBinding> (brainRenderOp, 2, agy3d);
    graph.CreateInputConnection<RG::ImageInputBinding> (brainRenderOp, 3, matcap);
    graph.CreateOutputConnection (brainRenderOp, 0, presented);


    // ========================= GRAPH RESOURCE SETUP =========================

    graph.Compile (s);

    matcap.CopyTransitionTransfer (ReadImage (PROJECT_ROOT / "src" / "VizHF" / "matcap.jpg", 4));

    std::vector<uint8_t> rawBrainData = ReadImage (PROJECT_ROOT / "src" / "VizHF" / "brain.jpg", 1);

    std::vector<uint8_t> transformedBrainData (256 * 256 * 256);

    auto BrainDataIndexMapping = [] (uint32_t oirignalDataIndex) {
        const uint32_t x = oirignalDataIndex % 4096;
        const uint32_t y = oirignalDataIndex / 4096;

        const uint32_t row        = x % 256;
        const uint32_t column     = y % 256;
        const uint32_t sliceIndex = 16 * (y / 256) + (x / 256);

        return sliceIndex * 256 * 256 + row + column * 256;
    };

    {
        Utils::DebugTimerLogger l ("transforming volume data");
        Utils::TimerScope       s (l);
        MultithreadedFunction   d ([&] (uint32_t threadCount, uint32_t threadIndex) {
            const uint32_t pixelCount = 4096 * 4096;
            for (uint32_t bidx = pixelCount / threadCount * threadIndex; bidx < pixelCount / threadCount * (threadIndex + 1); ++bidx) {
                transformedBrainData[BrainDataIndexMapping (bidx)] = rawBrainData[bidx];
            }
        });
    }

    agy3d.CopyTransitionTransfer (transformedBrainData);


    // ========================= RENDERING =========================

    RG::SynchronizedSwapchainGraphRenderer renderer (graph, swapchain);

    enum class DisplayMode : uint32_t {
        Feladat1 = 1,
        Feladat2,
        Feladat3,
        Feladat4,
        Feladat5,
        Feladat6,
    };

    std::cout << "1: Befoglalo" << std::endl;
    std::cout << "2: Szintfelulet (Phong)" << std::endl;
    std::cout << "3: Matcap" << std::endl;
    std::cout << "4: Hagymahej" << std::endl;
    std::cout << "5: Arnyek" << std::endl;

    DisplayMode currentDisplayMode = DisplayMode::Feladat2;

    bool quit = false;

    window->events.keyPressed += [&] (uint32_t key) {
        constexpr uint8_t ESC_CODE = 27;
        if (key == ESC_CODE) {
            // window->ToggleFullscreen ();
            quit = true;
        }
        if (key == 'R') {
            std::cout << "waiting for device... " << std::endl;
            vkDeviceWaitIdle (graph.GetGraphSettings ().GetDevice ());
            vkQueueWaitIdle (graph.GetGraphSettings ().queue);
            sp->Reload ();
            renderer.Recreate ();
        }
        switch (key) {
            case '1': currentDisplayMode = DisplayMode::Feladat1; break;
            case '2': currentDisplayMode = DisplayMode::Feladat2; break;
            case '3': currentDisplayMode = DisplayMode::Feladat3; break;
            case '4': currentDisplayMode = DisplayMode::Feladat4; break;
            case '5': currentDisplayMode = DisplayMode::Feladat5; break;
        }
    };

    refl.vertUbos["Camera"]["VP"] = glm::mat4 (1.f);
    refl.fragUbos["Camera"]["VP"] = glm::mat4 (1.f);

    renderer.preSubmitEvent += [&] (uint32_t frameIndex, uint64_t deltaNs) {
        TimePoint delta (deltaNs);

        const float dt = delta.AsSeconds ();

        cameraControl.UpdatePosition (dt);

        {
            refl.vertUbos["Camera"]["viewMatrix"] = c.GetViewMatrix ();
            //refl.vertUbos["Camera"]["VP"]           = c.GetViewProjectionMatrix ();
            refl.vertUbos["Camera"]["rayDirMatrix"] = c.GetRayDirMatrix ();
            refl.vertUbos["Camera"]["camPosition"]  = c.GetPosition ();
            refl.vertUbos["Camera"]["viewDir"]      = c.GetViewDirection ();
            refl.vertUbos["Camera"]["displayMode"]  = static_cast<uint32_t> (currentDisplayMode);

            refl.fragUbos["Camera"]["viewMatrix"] = c.GetViewMatrix ();
            //refl.fragUbos["Camera"]["VP"]           = c.GetViewProjectionMatrix ();
            refl.fragUbos["Camera"]["rayDirMatrix"] = c.GetRayDirMatrix ();
            refl.fragUbos["Time"]["time"]           = static_cast<float> (TimePoint::SinceApplicationStart ().AsSeconds ());
            refl.fragUbos["Camera"]["position"]     = c.GetPosition ();
            auto s                                  = refl.fragUbos["Camera"]["position"].As<glm::vec3> ();
            refl.fragUbos["Camera"]["viewDir"]      = c.GetViewDirection ();
            refl.fragUbos["Camera"]["displayMode"]  = static_cast<uint32_t> (currentDisplayMode);
        }

        refl.Update (frameIndex);
    };

    window->DoEventLoop (renderer.GetConditionalDrawCallback ([&] { return quit; }));
}
