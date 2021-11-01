#include "MovieExporter.hpp"

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
        std::cout << "File does not exist." << std::endl;
        return EXIT_FAILURE;
    }

    std::unique_ptr<RG::VulkanEnvironment> env = std::make_unique<RG::VulkanEnvironment> (RG::defaultDebugCallback, RG::GetGLFWInstanceExtensions (), std::vector<const char*> { VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME });

    std::unique_ptr<SequenceAdapter> sequenceAdapter = Gears::GetSequenceAdapterFromPyx (*env, sequencePath);
    if (GVK_ERROR (sequenceAdapter == nullptr)) {
        spdlog::error ("Failed to load sequence.");
        return EXIT_FAILURE;
    }

    if (exportMovieFlag.IsFlagOn ()) {

        spdlog::info ("Exporting video...");

        const size_t videoWidth  = 1280;
        const size_t videoHeight = 720;

        std::shared_ptr<RG::Presentable> pres = std::make_unique<RG::Presentable> (*env, std::make_unique<RG::HiddenGLFWWindow> (videoWidth, videoHeight), std::make_unique<GVK::DefaultSwapchainSettingsSingleImage> ());

        sequenceAdapter->SetCurrentPresentable (pres);
        
        const char* filename = "test.mp4";
        
        {
            MovieExporter movie { std::filesystem::current_path () / filename, { videoWidth, videoHeight, 60, 2 } };

            std::vector<uint8_t> framerawRGBA (videoWidth * videoHeight * 4, 0);
            std::vector<uint8_t> framerawRGB (videoWidth * videoHeight * 3, 0);

            for (size_t frameIndex = 1; frameIndex < 10*60; ++frameIndex) {
                sequenceAdapter->RenderFrameIndex (frameIndex);
                sequenceAdapter->Wait ();
                std::vector<std::unique_ptr<GVK::InheritedImage>> imgs = pres->GetSwapchain ().GetImageObjects ();

                GVK::ImageData::FillBuffer (*env->deviceExtra, *imgs[0], 0, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, framerawRGBA.data (), framerawRGBA.size ());

                size_t indexer = 0;
                for (size_t i = 0; i < framerawRGBA.size (); i += 4) {
                    framerawRGB[indexer++] = framerawRGBA[i + 0];
                    framerawRGB[indexer++] = framerawRGBA[i + 1];
                    framerawRGB[indexer++] = framerawRGBA[i + 2];
                }

                movie.PushFrame (framerawRGB);
            }
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
