#ifndef VULKANTESTENVIRONMENT_HPP
#define VULKANTESTENVIRONMENT_HPP

#include "Ptr.hpp"
#include "TerminalColors.hpp"
#include "VulkanUtils.hpp"
#include "VulkanWrapper.hpp"

#include "gtest/gtest.h"


std::optional<uint32_t> dontCare (VkPhysicalDevice, VkSurfaceKHR, const std::vector<VkQueueFamilyProperties>&)
{
    return std::nullopt;
}

std::optional<uint32_t> acceptPresentSupport (VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, const std::vector<VkQueueFamilyProperties>& queueFamilies)
{
    if (surface == VK_NULL_HANDLE) {
        return std::nullopt;
    }

    uint32_t i = 0;
    for (const VkQueueFamilyProperties& queueFamily : queueFamilies) {
        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR (physicalDevice, i, surface, &presentSupport);
        if (presentSupport) {
            return i;
        }

        i++;
    }

    return std::nullopt;
}


auto acceptWithFlag (VkQueueFlagBits flagbits)
{
    return [=] (VkPhysicalDevice, VkSurfaceKHR, const std::vector<VkQueueFamilyProperties>& props) -> std::optional<uint32_t> {
        uint32_t i = 0;
        for (const auto& p : props) {
            if (p.queueFlags & flagbits) {
                return i;
            }
            ++i;
        }
        return std::nullopt;
    };
}


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
class TestInstance final : public Instance {
public:
    TestInstance ()
        : Instance ({VK_EXT_DEBUG_UTILS_EXTENSION_NAME}, {"VK_LAYER_KHRONOS_validation"})
    {
    }

    USING_PTR (TestInstance);
};

// common for all test cases
class TestEnvironment final {
public:
    DebugUtilsMessenger messenger;
    PhysicalDevice      physicalDevice;

    TestEnvironment (Instance& instance, VkSurfaceKHR surface = VK_NULL_HANDLE)
        : messenger (instance, testDebugCallback, DebugUtilsMessenger::noPerformance)
        , physicalDevice (instance, surface, {}, {acceptWithFlag (VK_QUEUE_GRAPHICS_BIT), acceptPresentSupport, dontCare, dontCare})
    {
        uint32_t apiVersion;
        vkEnumerateInstanceVersion (&apiVersion);
        std::cout << "instance api version: " << GetVersionString (apiVersion) << std::endl;

        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties (physicalDevice, &deviceProperties);

        std::cout << "physical device api version: " << GetVersionString (deviceProperties.apiVersion) << std::endl;
        std::cout << "physical device driver version: " << GetVersionString (deviceProperties.driverVersion) << std::endl;
    }

    USING_PTR (TestEnvironment);
};


// unique for each test case
class TestCase final {
public:
    Device      device;
    Queue       queue;
    CommandPool commandPool;

    TestCase (const TestEnvironment& env, const std::vector<const char*>& deviceExtenions = {})
        : device (env.physicalDevice, {*env.physicalDevice.queueFamilies.graphics}, deviceExtenions)
        , queue (device, *env.physicalDevice.queueFamilies.graphics)
        , commandPool (device, *env.physicalDevice.queueFamilies.graphics)
    {
    }

    USING_PTR (TestCase);
};


class VulkanTestEnvironment : public ::testing::Test {
protected:
    static TestInstance::U    instance;
    static TestEnvironment::U env;
    TestCase::U               testCase;

    PhysicalDevice& GetPhysicalDevice () { return env->physicalDevice; }
    Device&         GetDevice () { return testCase->device; }
    CommandPool&    GetCommandPool () { return testCase->commandPool; }
    Queue&          GetQueue () { return testCase->queue; }

    static void SetUpTestSuite ()
    {
        instance = TestInstance::Create ();
        env      = TestEnvironment::Create (*instance);
    }

    static void TearDownTestSuite ()
    {
        env.reset ();
        instance.reset ();
    }

    virtual void SetUp () override
    {
        testCase = TestCase::Create (*env);
    }

    virtual void TearDown () override
    {
        testCase.reset ();
    }

    void CompareImages (const std::string& imageName, const Image& image, std::optional<VkImageLayout> transitionFrom = std::nullopt)
    {
        if (transitionFrom.has_value ()) {
            TransitionImageLayout (GetDevice (), GetQueue (), GetCommandPool (), image, *transitionFrom, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
        }

        const bool imagesMatch = AreImagesEqual (GetDevice (), GetQueue (), GetCommandPool (), image, PROJECT_ROOT / ("expected_" + imageName + ".png"));

        EXPECT_TRUE (imagesMatch);

        if (!imagesMatch) {
            SaveImageToFileAsync (GetDevice (), GetQueue (), GetCommandPool (), image, PROJECT_ROOT / ("actual_" + imageName + ".png")).join ();
        }
    }
};

TestEnvironment::U VulkanTestEnvironment::env;
TestInstance::U    VulkanTestEnvironment::instance;

#endif