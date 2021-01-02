#include "Assert.hpp"
#include "Camera.hpp"
#include "FullscreenQuad.hpp"
#include "GLFWWindow.hpp"
#include "GraphRenderer.hpp"
#include "GraphSettings.hpp"
#include "ImageData.hpp"
#include "MultithreadedFunction.hpp"
#include "Noncopyable.hpp"
#include "Operation.hpp"
#include "RenderGraph.hpp"
#include "Resource.hpp"
#include "SDLWindow.hpp"
#include "Time.hpp"
#include "Timer.hpp"
#include "Utils.hpp"
#include "VulkanEnvironment.hpp"
#include "GraphSettings.hpp"
#include "Operation.hpp"
#include "Resource.hpp"

// from VulkanWrapper
#include "VulkanWrapper.hpp"

#include "DeviceExtra.hpp"
#include "ShaderReflection.hpp"

#include "CameraControl.hpp"
#include "UniformReflection.hpp"

#include <array>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <iterator>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <vector>

#include "GoogleTestEnvironment.hpp"
#include "UUID.hpp"
#include "UniformView.hpp"

#include "glmlib.hpp"

#include <unordered_map>


using VizHFTests = HiddenWindowGoogleTestEnvironment;

const std::filesystem::path ShadersFolder = PROJECT_ROOT / "src" / "VizHF" / "shaders";

TEST_F (VizHFTests, HF1)
{
    DeviceExtra& device        = *env->deviceExtra;
    CommandPool& commandPool   = *env->commandPool;
    Queue&       graphicsQueue = *env->graphicsQueue;

    Swapchain& swapchain = presentable->GetSwapchain ();

    Camera        c (glm::vec3 (-1, 0, 0.5f), glm::vec3 (1, 0.0f, 0), window->GetAspectRatio ());
    CameraControl cameraControl (c, window->events);

    RG::GraphSettings s (device, swapchain.GetImageCount ());
    RG::RenderGraph   graph;


    // ========================= GRAPH OPERATIONS =========================

    ShaderPipelineP sp = ShaderPipeline::CreateShared (device);
    sp->SetShadersFromSourceFiles ({
        ShadersFolder / "brain.vert",
        ShadersFolder / "brain.frag",
    });

    RG::RenderOperationP brainRenderOp = RG::RenderOperation::Create (FullscreenQuad::CreateShared (device), sp);


    // ========================= GRAPH RESOURCES =========================

    RG::SwapchainImageResourceP presented = RG::SwapchainImageResource::Create (*presentable);
    RG::ReadOnlyImageResourceP  matcap    = RG::ReadOnlyImageResource::Create (VK_FORMAT_R8G8B8A8_SRGB, 512, 512);
    RG::ReadOnlyImageResourceP  agy3d     = RG::ReadOnlyImageResource::Create (VK_FORMAT_R8_SRGB, 256, 256, 256);

    s.connectionSet.Add (brainRenderOp, presented,
                         RG::OutputBinding::Create (0,
                                                    presented->GetFormatProvider (),
                                                    presented->GetFinalLayout (),
                                                    VK_ATTACHMENT_LOAD_OP_CLEAR,
                                                    VK_ATTACHMENT_STORE_OP_STORE));

    s.connectionSet.Add (agy3d, brainRenderOp, RG::ImageInputBinding::Create (2, *agy3d));
    s.connectionSet.Add (matcap, brainRenderOp, RG::ImageInputBinding::Create (3, *matcap));

    RG::UniformReflection r (s.connectionSet);

    // ========================= GRAPH RESOURCE SETUP =========================

    graph.Compile (s);

    matcap->CopyTransitionTransfer (ImageData (PROJECT_ROOT / "src" / "VizHF" / "matcap.jpg").data);

    std::vector<uint8_t> rawBrainData = ImageData (PROJECT_ROOT / "src" / "VizHF" / "brain.jpg", 1).data;

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

    agy3d->CopyTransitionTransfer (transformedBrainData);


    // ========================= RENDERING =========================

    RG::SynchronizedSwapchainGraphRenderer renderer (s, swapchain);

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

    EventObserver obs;
    obs.Observe (window->events.keyPressed, [&] (uint32_t key) {
        constexpr uint8_t ESC_CODE = 27;
        if (key == ESC_CODE) {
            // window->ToggleFullscreen ();
            quit = true;
        }
        if (key == 'R') {
            std::cout << "waiting for device... " << std::endl;
            vkDeviceWaitIdle (s.GetDevice ());
            vkQueueWaitIdle (s.GetDevice ().GetGraphicsQueue ());
            sp->Reload ();
            renderer.Recreate (graph);
        }
        switch (key) {
            case '1': currentDisplayMode = DisplayMode::Feladat1; break;
            case '2': currentDisplayMode = DisplayMode::Feladat2; break;
            case '3': currentDisplayMode = DisplayMode::Feladat3; break;
            case '4': currentDisplayMode = DisplayMode::Feladat4; break;
            case '5': currentDisplayMode = DisplayMode::Feladat5; break;
        }
    });

    r[*brainRenderOp][ShaderKind::Vertex]["Camera"]["VP"]   = glm::mat4 (1.f);
    r[*brainRenderOp][ShaderKind::Fragment]["Camera"]["VP"] = glm::mat4 (1.f);

    obs.Observe (renderer.preSubmitEvent, [&] (RG::RenderGraph&, uint32_t frameIndex, uint64_t deltaNs) {
        TimePoint delta (deltaNs);

        const float dt = delta.AsSeconds ();

        cameraControl.UpdatePosition (dt);

        {
            r[*brainRenderOp][ShaderKind::Vertex]["Camera"]["viewMatrix"]   = c.GetViewMatrix ();
            r[*brainRenderOp][ShaderKind::Vertex]["Camera"]["rayDirMatrix"] = c.GetRayDirMatrix ();
            r[*brainRenderOp][ShaderKind::Vertex]["Camera"]["camPosition"]  = c.GetPosition ();
            r[*brainRenderOp][ShaderKind::Vertex]["Camera"]["viewDir"]      = c.GetViewDirection ();
            r[*brainRenderOp][ShaderKind::Vertex]["Camera"]["displayMode"]  = static_cast<uint32_t> (currentDisplayMode);

            r[*brainRenderOp][ShaderKind::Fragment]["Camera"]["viewMatrix"]  = c.GetViewMatrix ();
            r[*brainRenderOp][ShaderKind::Fragment]["Camera"]["viewMatrix"]  = c.GetRayDirMatrix ();
            r[*brainRenderOp][ShaderKind::Fragment]["Time"]["time"]          = static_cast<float> (TimePoint::SinceApplicationStart ().AsSeconds ());
            r[*brainRenderOp][ShaderKind::Fragment]["Camera"]["position"]    = c.GetPosition ();
            r[*brainRenderOp][ShaderKind::Fragment]["Camera"]["viewDir"]     = c.GetViewDirection ();
            r[*brainRenderOp][ShaderKind::Fragment]["Camera"]["displayMode"] = static_cast<uint32_t> (currentDisplayMode);
        }

        r.Flush (frameIndex);
    });

    renderer.RenderNextRecreatableFrame (graph);
    renderer.RenderNextRecreatableFrame (graph);
    renderer.RenderNextRecreatableFrame (graph);
    renderer.RenderNextRecreatableFrame (graph);
    renderer.RenderNextRecreatableFrame (graph);
    renderer.RenderNextRecreatableFrame (graph);

    ImageData sw (GetDeviceExtra (), *swapchain.GetImageObjects ()[0], 0, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
    sw.ConvertBGRToRGB ();
    sw.SaveTo (PROJECT_ROOT / "asd.png");
}
