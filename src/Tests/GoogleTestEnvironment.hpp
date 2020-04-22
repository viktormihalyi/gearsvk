#include "VulkanEnvironment.hpp"

#include "gtest/gtest.h"


static void gtestDebugCallback (VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
                                VkDebugUtilsMessageTypeFlagsEXT             messageType,
                                const VkDebugUtilsMessengerCallbackDataEXT* callbackData)
{
    using namespace TerminalColors;
    std::cout << RED << "validation layer: "
              << YELLOW << callbackData->pMessageIdName << ": "
              << RESET << callbackData->pMessage
              << std::endl
              << std::endl;
    FAIL ();
}

class EmptyTestEnvironment : public ::testing::Test {
protected:
    virtual void SetUp () {}
    virtual void TearDown () {}
};

class GoogleTestEnvironment : public ::testing::Test {
protected:
    Window::U            window;
    VulkanEnvironment::U env;

    PhysicalDevice& GetPhysicalDevice () { return *env->physicalDevice; }
    Device&         GetDevice () { return *env->device; }
    CommandPool&    GetCommandPool () { return *env->commandPool; }
    Queue&          GetGraphicsQueue () { return *env->graphicsQueue; }
    Swapchain&      GetSwapchain () { return *env->swapchain; }

    virtual ~GoogleTestEnvironment () {}

    virtual void SetUp ()    = 0;
    virtual void TearDown () = 0;

    void CompareImages (const std::string& imageName, const Image& image, std::optional<VkImageLayout> transitionFrom = std::nullopt)
    {
        if (transitionFrom.has_value ()) {
            TransitionImageLayout (GetDevice (), GetGraphicsQueue (), GetCommandPool (), image, *transitionFrom, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
        }

        const bool imagesMatch = AreImagesEqual (GetDevice (), GetGraphicsQueue (), GetCommandPool (), image, PROJECT_ROOT / (imageName + "_reference.png"));

        EXPECT_TRUE (imagesMatch);

        if (!imagesMatch) {
            SaveImageToFileAsync (GetDevice (), GetGraphicsQueue (), GetCommandPool (), image, PROJECT_ROOT / (imageName + ".png")).join ();
        }
    }
};


// no window, swapchain, surface
class HeadlessGoogleTestEnvironment : public GoogleTestEnvironment {
protected:
    virtual void SetUp () override
    {
        env = DebugVulkanEnvironment::Create (std::nullopt, gtestDebugCallback);
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
        window = GLFWWindow::Create ();
        env    = DebugVulkanEnvironment::Create (*window, gtestDebugCallback);
    }

    virtual void TearDown () override
    {
        env.reset ();
        window.reset ();
    }
};


class HiddenWindowGoogleTestEnvironment : public GoogleTestEnvironment {
protected:
    virtual void SetUp () override
    {
        window = HiddenGLFWWindow::Create ();
        env    = DebugVulkanEnvironment::Create (*window, gtestDebugCallback);
    }

    virtual void TearDown () override
    {
        env.reset ();
        window.reset ();
    }
};
