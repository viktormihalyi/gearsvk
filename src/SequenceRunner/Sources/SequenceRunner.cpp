#include "RenderGraph/Window/GLFWWindow.hpp"
#include "GearsPYD/GearsAPIv2.hpp"
#include "RenderGraph/GraphRenderer.hpp"
#include "RenderGraph/RenderGraph.hpp"
#include "RenderGraph/VulkanEnvironment.hpp"
#include "Sequence/SequenceAdapter.hpp"
#include "Sequence/StimulusAdapter.hpp"

#include "Utils/BuildType.hpp"
#include "Utils/CommandLineFlag.hpp"
#include "Utils/SetupLogger.hpp"

#include "spdlog/spdlog.h"

#include <iostream>

int main (int argc, char** argv)
{
    Utils::SetupLogger ();

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

    try {
        sequenceAdapter->RenderFullOnExternalWindow ();
    } catch (std::exception& ex) {
        spdlog::error ("Error occurred during display.\n{}", ex.what ());
        return EXIT_FAILURE;
    }

    sequenceAdapter->Wait ();

    return EXIT_SUCCESS;
}
