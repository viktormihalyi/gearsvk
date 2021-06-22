#include "GoogleTestEnvironment.hpp"

#include "GearsVk/Window/GLFWWindow.hpp"
#include "VulkanWrapper/Utils/ImageData.hpp"
#include "VulkanWrapper/Surface.hpp"
#include "Utils/TerminalColors.hpp"
#include "GearsVk/VulkanEnvironment.hpp"
#include "VulkanWrapper/VulkanWrapper.hpp"

using namespace GVK;


const std::filesystem::path ReferenceImagesFolder = PROJECT_ROOT / "TestData" / "ReferenceImages";
const std::filesystem::path TempFolder            = PROJECT_ROOT / "temp";


void gtestDebugCallback (VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
                         VkDebugUtilsMessageTypeFlagsEXT             messageType,
                         const VkDebugUtilsMessengerCallbackDataEXT* callbackData)
{
    using namespace TerminalColors;
    if (messageSeverity > 1) {
        std::cout << RED << "validation layer: "
                  << YELLOW << callbackData->pMessageIdName << ": "
                  << RESET << callbackData->pMessage
                  << std::endl
                  << std::endl;
        FAIL ();
    }
}


PhysicalDevice& GoogleTestEnvironmentBase::GetPhysicalDevice ()
{
    return *env->physicalDevice;
}


Device& GoogleTestEnvironmentBase::GetDevice ()
{
    return *env->device;
}


CommandPool& GoogleTestEnvironmentBase::GetCommandPool ()
{
    return *env->commandPool;
}


Queue& GoogleTestEnvironmentBase::GetGraphicsQueue ()
{
    return *env->graphicsQueue;
}


DeviceExtra& GoogleTestEnvironmentBase::GetDeviceExtra ()
{
    return *env->deviceExtra;
}


Window& GoogleTestEnvironmentBase::GetWindow ()
{
    return *window;
}


Swapchain& GoogleTestEnvironmentBase::GetSwapchain ()
{
    return presentable->GetSwapchain ();
}


void GoogleTestEnvironmentBase::CompareImages (const std::string& imageName, const Image& image, std::optional<VkImageLayout> transitionFrom)
{
    if (transitionFrom.has_value ()) {
        TransitionImageLayout (GetDeviceExtra (), image, *transitionFrom, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    }

    const bool imagesMatch = AreImagesEqual (GetDeviceExtra (), image, ReferenceImagesFolder / (imageName + "_reference.png"));

    EXPECT_TRUE (imagesMatch);

    if (!imagesMatch) {
        const std::filesystem::path outPath = TempFolder / (imageName + "_actual.png");
        std::cout << "Saving " << outPath.string () << "..." << std::endl;
        SaveImageToFileAsync (GetDeviceExtra (), image, outPath).join ();
    }
}


void GoogleTestEnvironmentBase::CompareImages (const std::string& name, const ImageData& actualImage)
{
    const ImageData referenceImage (ReferenceImagesFolder / (name + "_Reference.png"));

    const ImageData::ComparisonResult comparison = referenceImage.CompareTo (actualImage);

    EXPECT_TRUE (comparison.equal);

    if (!comparison.equal) {
        const std::filesystem::path outPathRef = TempFolder / (name + "_Reference.png");
        std::cout << "Saving " << outPathRef.string () << "..." << std::endl;
        referenceImage.SaveTo (outPathRef);

        const std::filesystem::path outPath = TempFolder / (name + "_Actual.png");
        std::cout << "Saving " << outPath.string () << "..." << std::endl;
        actualImage.SaveTo (outPath);

        if (GVK_VERIFY (comparison.diffImage != nullptr)) {
            const std::filesystem::path outPathDiff = TempFolder / (name + "_Diff.png");
            std::cout << "Saving " << outPathDiff.string () << "..." << std::endl;
            comparison.diffImage->SaveTo (outPathDiff);
        }
    }
}


void HeadlessGoogleTestEnvironment::SetUp ()
{
    env = std::make_unique<VulkanEnvironment> (gtestDebugCallback);
}


void HeadlessGoogleTestEnvironment::TearDown ()
{
    env.reset ();
}


void ShownWindowGoogleTestEnvironment::SetUp ()
{
    window      = std::make_unique<GLFWWindow> ();
    env         = std::make_unique<VulkanEnvironment> (gtestDebugCallback);
    presentable = std::make_unique<Presentable> (*env, *window);
}


void ShownWindowGoogleTestEnvironment::TearDown ()
{
    presentable.reset ();
    env.reset ();
    window.reset ();
}


void HiddenWindowGoogleTestEnvironment::SetUp ()
{
    window      = std::make_unique<HiddenGLFWWindow> ();
    env         = std::make_unique<VulkanEnvironment> (gtestDebugCallback);
    presentable = std::make_unique<Presentable> (*env, *window);
}


void HiddenWindowGoogleTestEnvironment::TearDown ()
{
    presentable.reset ();
    env.reset ();
    window.reset ();
}
