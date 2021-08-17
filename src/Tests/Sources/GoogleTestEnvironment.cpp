#include "GoogleTestEnvironment.hpp"

#include "RenderGraph/Window/GLFWWindow.hpp"
#include "VulkanWrapper/Utils/ImageData.hpp"
#include "VulkanWrapper/Surface.hpp"
#include "RenderGraph/VulkanEnvironment.hpp"
#include "VulkanWrapper/VulkanWrapper.hpp"


const std::filesystem::path ReferenceImagesFolder = std::filesystem::current_path () / "TestData" / "ReferenceImages";
const std::filesystem::path TempFolder            = std::filesystem::current_path () / "temp";


void gtestDebugCallback (VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
                         VkDebugUtilsMessageTypeFlagsEXT             messageType,
                         const VkDebugUtilsMessengerCallbackDataEXT* callbackData)
{
    GVK::defaultDebugCallback (messageSeverity, messageType, callbackData);

    if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) {
        EXPECT_TRUE (false);
    }
}


GVK::PhysicalDevice& GoogleTestEnvironmentBase::GetPhysicalDevice ()
{
    return *env->physicalDevice;
}


GVK::Device& GoogleTestEnvironmentBase::GetDevice ()
{
    return *env->device;
}


GVK::CommandPool& GoogleTestEnvironmentBase::GetCommandPool ()
{
    return *env->commandPool;
}


GVK::Queue& GoogleTestEnvironmentBase::GetGraphicsQueue ()
{
    return *env->graphicsQueue;
}


GVK::DeviceExtra& GoogleTestEnvironmentBase::GetDeviceExtra ()
{
    return *env->deviceExtra;
}


GVK::Window& GoogleTestEnvironmentBase::GetWindow ()
{
    return *window;
}


GVK::Swapchain& GoogleTestEnvironmentBase::GetSwapchain ()
{
    return presentable->GetSwapchain ();
}


void GoogleTestEnvironmentBase::CompareImages (const std::string& imageName, const GVK::Image& image, std::optional<VkImageLayout> transitionFrom)
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


void GoogleTestEnvironmentBase::CompareImages (const std::string& name, const GVK::ImageData& actualImage)
{
    GVK_ASSERT (std::filesystem::exists (ReferenceImagesFolder / (name + ".png")));

    const GVK::ImageData referenceImage (ReferenceImagesFolder / (name + ".png"));

    const GVK::ImageData::ComparisonResult comparison = referenceImage.CompareTo (actualImage);

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
    env = std::make_unique<GVK::VulkanEnvironment> (gtestDebugCallback);
}


void HeadlessGoogleTestEnvironment::TearDown ()
{
    env.reset ();
}


void ShownWindowGoogleTestEnvironment::SetUp ()
{
    window      = std::make_unique<GVK::GLFWWindow> ();
    env         = std::make_unique<GVK::VulkanEnvironment> (gtestDebugCallback, GVK::GetGLFWInstanceExtensions (), std::vector<const char*> { VK_KHR_SWAPCHAIN_EXTENSION_NAME });
    presentable = std::make_unique<GVK::Presentable> (*env, *window, std::make_unique<GVK::DefaultSwapchainSettings> ());
}


void ShownWindowGoogleTestEnvironment::TearDown ()
{
    presentable.reset ();
    env.reset ();
    window.reset ();
}


void HiddenWindowGoogleTestEnvironment::SetUp ()
{
    window      = std::make_unique<GVK::HiddenGLFWWindow> ();
    env         = std::make_unique<GVK::VulkanEnvironment> (gtestDebugCallback, GVK::GetGLFWInstanceExtensions (), std::vector<const char*> { VK_KHR_SWAPCHAIN_EXTENSION_NAME });
    presentable = std::make_unique<GVK::Presentable> (*env, *window, std::make_unique<GVK::DefaultSwapchainSettings> ());
}


void HiddenWindowGoogleTestEnvironment::TearDown ()
{
    presentable.reset ();
    env.reset ();
    window.reset ();
}
