#ifndef GOOGLETESTENVIRONMENT_HPP
#define GOOGLETESTENVIRONMENT_HPP

#include "GLFWWindow.hpp"
#include "ImageData.hpp"
#include "TerminalColors.hpp"
#include "VulkanEnvironment.hpp"

#include "gtest/gtest.h"

#include <filesystem>

#include <vulkan/vulkan.h>


const std::filesystem::path ReferenceImagesFolder = PROJECT_ROOT / "src" / "Tests" / "ReferenceImages";
const std::filesystem::path TempFolder            = PROJECT_ROOT / "temp";


static void gtestDebugCallback (VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
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


class EmptyTestEnvironment : public ::testing::Test {
protected:
    virtual void SetUp () {}
    virtual void TearDown () {}
};


class GoogleTestEnvironment : public ::testing::Test {
protected:
    VulkanEnvironmentU env;
    WindowU            window;
    Ptr<Presentable>     presentable;

    PhysicalDevice& GetPhysicalDevice () { return *env->physicalDevice; }
    Device&         GetDevice () { return *env->device; }
    CommandPool&    GetCommandPool () { return *env->commandPool; }
    Queue&          GetGraphicsQueue () { return *env->graphicsQueue; }
    DeviceExtra&    GetDeviceExtra () { return *env->deviceExtra; }
    Window&         GetWindow () { return *window; }
    Swapchain&      GetSwapchain () { return presentable->GetSwapchain (); }

    virtual ~GoogleTestEnvironment () = default;

    virtual void SetUp ()    = 0;
    virtual void TearDown () = 0;

    void CompareImages (const std::string& imageName, const Image& image, std::optional<VkImageLayout> transitionFrom = std::nullopt)
    {
        if (transitionFrom.has_value ()) {
            TransitionImageLayout (GetDeviceExtra (), image, *transitionFrom, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
        }

        const bool imagesMatch = AreImagesEqual (GetDeviceExtra (), image, ReferenceImagesFolder / (imageName + "_reference.png"));

        EXPECT_TRUE (imagesMatch);

        if (!imagesMatch) {
            SaveImageToFileAsync (GetDeviceExtra (), image, TempFolder / (imageName + "_actual.png")).join ();
        }
    }

    void CompareImages (const std::string& name, const ImageData& actualImage)
    {
        const ImageData referenceImage (ReferenceImagesFolder / (name + "_Reference.png"));

        const bool imagesMatch = referenceImage == actualImage;

        EXPECT_TRUE (imagesMatch);

        if (!imagesMatch) {
            actualImage.SaveTo (TempFolder / (name + "_Actual.png"));
        }
    }
};


// no window, swapchain, surface
class HeadlessGoogleTestEnvironment : public GoogleTestEnvironment {
protected:
    virtual void SetUp () override
    {
        env = VulkanEnvironment::Create (gtestDebugCallback);
    }

    virtual void TearDown () override
    {
        env.reset ();
    }
};


class ShownWindowGoogleTestEnvironment : public GoogleTestEnvironment {
protected:
    virtual void SetUp () override
    {
        window      = GLFWWindow::Create ();
        env         = VulkanEnvironment::Create (gtestDebugCallback);
        presentable = Presentable::Create (*env, *window);
    }

    virtual void TearDown () override
    {
        presentable.reset ();
        env.reset ();
        window.reset ();
    }
};


class HiddenWindowGoogleTestEnvironment : public GoogleTestEnvironment {
protected:
    virtual void SetUp () override
    {
        window      = HiddenGLFWWindow::Create ();
        env         = VulkanEnvironment::Create (gtestDebugCallback);
        presentable = Presentable::Create (*env, *window);
    }

    virtual void TearDown () override
    {
        presentable.reset ();
        env.reset ();
        window.reset ();
    }
};


#endif