#ifndef VULKANTESTENVIRONMENT_HPP
#define VULKANTESTENVIRONMENT_HPP

#include "Ptr.hpp"
#include "SDLWindow.hpp"
#include "TerminalColors.hpp"
#include "VulkanUtils.hpp"
#include "VulkanWrapper.hpp"

#include "gtest/gtest.h"


static void testDebugCallback (VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
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


// common for all test cases
class TestEnvironment final {
public:
    Instance::U            instance;
    DebugUtilsMessenger::U messenger;
    PhysicalDevice::U      physicalDevice;
    Device::U              device;
    Queue::U               graphicsQueue;
    CommandPool::U         commandPool;

    // surface and swapchain are created if a window is provided in the ctor
    Surface::U   surface;
    Swapchain::U swapchain;

    USING_PTR (TestEnvironment);

    TestEnvironment (std::vector<const char*> instanceExtensions, std::optional<WindowBase::Ref> window = std::nullopt)
    {
        if (window) {
            auto windowExtenions = window->get ().GetExtensions ();
            instanceExtensions.insert (instanceExtensions.end (), windowExtenions.begin (), windowExtenions.end ());
        }

        instance = Instance::Create (instanceExtensions, std::vector<const char*> {"VK_LAYER_KHRONOS_validation"});

        if (window) {
            surface = Surface::Create (*instance, window->get ().CreateSurface (*instance));
        }

        messenger = DebugUtilsMessenger::Create (*instance, testDebugCallback, DebugUtilsMessenger::noPerformance);

        VkSurfaceKHR physicalDeviceSurfaceHandle = ((surface != nullptr) ? surface->operator VkSurfaceKHR () : VK_NULL_HANDLE);

        physicalDevice = PhysicalDevice::Create (*instance, physicalDeviceSurfaceHandle, std::set<std::string> {});

        std::vector<const char*> deviceExtensions;

        if (window) {
            deviceExtensions.push_back (VK_KHR_SWAPCHAIN_EXTENSION_NAME);
        }

        device = Device::Create (*physicalDevice, std::vector<uint32_t> {*physicalDevice->GetQueueFamilies ().graphics}, deviceExtensions);

        graphicsQueue = Queue::Create (*device, *physicalDevice->GetQueueFamilies ().graphics);

        commandPool = CommandPool::Create (*device, *physicalDevice->GetQueueFamilies ().graphics);

        if (window) {
            swapchain = RealSwapchain::Create (*physicalDevice, *device, *surface);
        } else {
            swapchain = FakeSwapchain::Create (*device, *graphicsQueue, *commandPool, 512, 512);
        }

        if (false) {
            uint32_t apiVersion;
            vkEnumerateInstanceVersion (&apiVersion);
            std::cout << "instance api version: " << GetVersionString (apiVersion) << std::endl;

            VkPhysicalDeviceProperties deviceProperties;
            vkGetPhysicalDeviceProperties (*physicalDevice, &deviceProperties);

            std::cout << "physical device api version: " << GetVersionString (deviceProperties.apiVersion) << std::endl;
            std::cout << "physical device driver version: " << GetVersionString (deviceProperties.driverVersion) << std::endl;
        }
    }
};


class VulkanTestEnvironmentBase : public ::testing::Test {
protected:
    WindowBase::U      window;
    TestEnvironment::U env;

    PhysicalDevice& GetPhysicalDevice () { return *env->physicalDevice; }
    Device&         GetDevice () { return *env->device; }
    CommandPool&    GetCommandPool () { return *env->commandPool; }
    Queue&          GetGraphicsQueue () { return *env->graphicsQueue; }

    Swapchain& GetSwapchain ()
    {
        ASSERT (window != nullptr);
        return *env->swapchain;
    }

    virtual ~VulkanTestEnvironmentBase () {}

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
class HeadlessVulkanTestEnvironment : public VulkanTestEnvironmentBase {
protected:
    virtual void SetUp () override
    {
        env = TestEnvironment::Create (std::vector<const char*> {VK_EXT_DEBUG_UTILS_EXTENSION_NAME});
    }

    virtual void TearDown () override
    {
        env.reset ();
    }
};


class ShownWindowVulkanTestEnvironment : public VulkanTestEnvironmentBase {
protected:
    virtual void SetUp () override
    {
        window = SDLWindow::Create ();
        env    = TestEnvironment::Create (std::vector<const char*> {VK_EXT_DEBUG_UTILS_EXTENSION_NAME}, *window);
    }

    virtual void TearDown () override
    {
        env.reset ();
        window.reset ();
    }
};


class HiddenWindowVulkanTestEnvironment : public VulkanTestEnvironmentBase {
protected:
    virtual void SetUp () override
    {
        window = HiddenSDLWindow::Create ();
        env    = TestEnvironment::Create (std::vector<const char*> {VK_EXT_DEBUG_UTILS_EXTENSION_NAME}, *window);
    }

    virtual void TearDown () override
    {
        env.reset ();
        window.reset ();
    }
};

#endif