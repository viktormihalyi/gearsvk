#include "VideoExporter.hpp"

#include "RenderGraph/Window/GLFWWindow.hpp"
#include "GearsPYD/GearsAPIv2.hpp"
#include "RenderGraph/GraphRenderer.hpp"
#include "RenderGraph/RenderGraph.hpp"
#include "RenderGraph/VulkanEnvironment.hpp"
#include "Sequence/Sequence.h"
#include "Sequence/SequenceAdapter.hpp"
#include "Sequence/StimulusAdapter.hpp"

#include "VulkanWrapper/Utils/ImageData.hpp"

#include "Utils/BuildType.hpp"
#include "Utils/CommandLineFlag.hpp"
#include "Utils/SetupLogger.hpp"
#include "Utils/Assert.hpp"

#include "spdlog/spdlog.h"

#include <iostream>
#include <cstdlib>

#include <fmt/format.h>


static const Utils::CommandLineOnOffFlag exportMovieFlag { "--exportVideo" };


int main (int argc, char* argv[])
{
    spdlog::set_default_logger (Utils::GetLogger ());

    if (argc < 2) {
        std::cout << "Fist argument must be an absolute path of a sequence .pyx file." << std::endl;
        return EXIT_FAILURE;
    }

    if constexpr (IsDebugBuild) {
        std::cout << "Press enter to start." << std::endl;
        std::cin.ignore ();
    }

    Utils::CommandLineFlag::MatchAll (argc, argv, false);

    const std::string           sequencePathStr (argv[1]);
    const std::filesystem::path sequencePath (sequencePathStr);

    if (GVK_ERROR (!std::filesystem::exists (sequencePath))) {
        std::cout << "File does not exist: " << sequencePath.string () << std::endl;
        return EXIT_FAILURE;
    }

    std::unique_ptr<RG::VulkanEnvironment> env = std::make_unique<RG::VulkanEnvironment> (RG::defaultDebugCallback, RG::GetGLFWInstanceExtensions (), std::vector<const char*> { VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME });

    std::unique_ptr<SequenceAdapter> sequenceAdapter = Gears::GetSequenceAdapterFromPyx (*env, sequencePath);
    if (GVK_ERROR (sequenceAdapter == nullptr)) {
        spdlog::error ("Failed to load sequence.");
        return EXIT_FAILURE;
    }

    if (exportMovieFlag.IsFlagOn ()) {
        
        std::chrono::nanoseconds startNs;
        {
            const std::chrono::nanoseconds currentNs = std::chrono::duration_cast<std::chrono::nanoseconds> (std::chrono::system_clock::now ().time_since_epoch ());
            spdlog::info ("[VideExporter] Exporting video...");
            spdlog::info ("[VideExporter] Sequence duration: {} frames", sequenceAdapter->GetSequence ()->getDuration ());
            startNs = currentNs;
        }

        uint32_t videoWidth  = 1280;
        uint32_t videoHeight = 720;

        if (const char* envVar = std::getenv ("GVK_VIDEO_WIDTH")) {
            videoWidth = std::stoi (envVar);
        }
        if (const char* envVar = std::getenv ("GVK_VIDEO_HEIGHT")) {
            videoHeight = std::stoi (envVar);
        }

        std::string crf = "23";
        if (const char* envVar = std::getenv ("GVK_CRF")) {
            crf = std::string (envVar);
        }

        std::shared_ptr<RG::Presentable> pres = std::make_unique<RG::Presentable> (*env, std::make_unique<RG::HiddenGLFWWindow> (videoWidth, videoHeight), std::make_unique<GVK::DefaultSwapchainSettingsSingleImage> ());

        sequenceAdapter->SetCurrentPresentable (pres);
        
        const std::string filename = fmt::format ("{}_{}_{}_crf{}.mp4", sequencePath.stem ().string (), videoWidth, videoHeight, crf);

        spdlog::info ("[VideExporter] Video width: {}, height: {}, CRF value: {}", videoWidth, videoHeight, crf);

        const std::filesystem::path videoPath = std::filesystem::current_path () / filename;

        {
            const uint32_t fps     = 60;
            const uint32_t bitrate = 2000;

            VideoExporter movie { videoPath, VideoExporter::VideoSettings { videoWidth, videoHeight, fps, bitrate } };

            std::vector<uint8_t> framerawRGBA (videoWidth * videoHeight * 4, 0);
            
            std::vector<std::unique_ptr<GVK::InheritedImage>> imgs = pres->GetSwapchain ().GetImageObjects ();

            uint32_t lastPercentageLogged = 0;

            for (size_t frameIndex = 1; frameIndex < sequenceAdapter->GetSequence ()->getDuration (); ++frameIndex) {
                const double donePercentage = static_cast<double> (frameIndex) / sequenceAdapter->GetSequence ()->getDuration () * 100;
                const uint32_t donePercentageInt = static_cast<uint32_t> (donePercentage);

                if (donePercentageInt % 10 == 0 && lastPercentageLogged != donePercentageInt) {
                    spdlog::info ("[VideExporter] Exported {}%", donePercentageInt);
                    lastPercentageLogged = donePercentageInt;
                }

                sequenceAdapter->RenderFrameIndex (frameIndex);
                sequenceAdapter->Wait ();

                GVK::ImageData::FillBuffer (*env->deviceExtra, *imgs[0], 0, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, framerawRGBA.data (), framerawRGBA.size ());

                movie.PushFrame (framerawRGBA);
            }
        }

        {
            const std::chrono::nanoseconds currentNs = std::chrono::duration_cast<std::chrono::nanoseconds> (std::chrono::system_clock::now ().time_since_epoch ());
            spdlog::info ("[VideExporter] Exported file: \"{}\", size: {} bytes, took {} seconds",
                          videoPath.string (),
                          std::filesystem::file_size (videoPath),
                          (currentNs.count () - startNs.count ()) / 1e9);
        }

        return 0;
    }


    try {
        sequenceAdapter->RenderFullOnExternalWindow ();
    } catch (std::exception& ex) {
        spdlog::error ("Error occurred during display.\n{}", ex.what ());
        return EXIT_FAILURE;
    }

    sequenceAdapter->Wait ();

    return EXIT_SUCCESS;
}
