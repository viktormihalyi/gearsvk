#include "GLFWWindow.hpp"
#include "GearsAPIv2.hpp"
#include "GraphRenderer.hpp"
#include "RenderGraph.hpp"
#include "SequenceAdapter.hpp"
#include "VulkanEnvironment.hpp"


int main (int argc, char** argv)
{
    if (argc < 2) {
        std::cout << "Fist argument must be an absolute path of a sequence .pyx file." << std::endl;
        return EXIT_FAILURE;
    }

    if (argc > 2) {
        std::cout << "Unnecessary arguments." << std::endl;
        return EXIT_FAILURE;
    }

    const std::string           sequencePathStr (argv[1]);
    const std::filesystem::path sequencePath (sequencePathStr);

    if (GVK_ERROR (!std::filesystem::exists (sequencePath))) {
        std::cout << "File does not exist." << std::endl;
        return EXIT_FAILURE;
    }

    std::unique_ptr<GVK::VulkanEnvironment> env = std::make_unique<GVK::VulkanEnvironment> ();

    std::unique_ptr<SequenceAdapter> sequenceAdapter = Gears::GetSequenceAdapterFromPyx (*env, sequencePath);
    if (GVK_ERROR (sequenceAdapter == nullptr)) {
        std::cout << "Failed to load sequence." << std::endl;
        return EXIT_FAILURE;
    }

    sequenceAdapter->RenderFullOnExternalWindow ();
    sequenceAdapter->Wait ();

    return EXIT_SUCCESS;
}
