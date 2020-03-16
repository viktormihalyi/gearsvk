#ifndef VULKANTESTENVIRONMENT_HPP
#define VULKANTESTENVIRONMENT_HPP

#include "Ptr.hpp"
#include "VulkanWrapper.hpp"

#include "gtest/gtest.h"


auto acceptAnything = [] (const std::vector<VkQueueFamilyProperties>&) { return 0; };


auto acceptWithFlag = [] (VkQueueFlagBits flagbits) {
    return [&] (const std::vector<VkQueueFamilyProperties>& props) -> std::optional<uint32_t> {
        uint32_t i = 0;
        for (const auto& p : props) {
            if (p.queueFlags & flagbits) {
                return i;
            }
            ++i;
        }
        return std::nullopt;
    };
};


static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback (
    VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT             messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void*                                       pUserData)
{
    std::cout << "validation layer: " << pCallbackData->pMessageIdName << ": " << pCallbackData->pMessage << std::endl
              << std::endl;
    //std::cout << "validation layer" << std::endl;
    return VK_FALSE;
}


class TestEnvironment final {
public:
    Instance            instance;
    DebugUtilsMessenger messenger;
    PhysicalDevice      physicalDevice;

    TestEnvironment ()
        : instance ({VK_EXT_DEBUG_UTILS_EXTENSION_NAME}, {"VK_LAYER_KHRONOS_validation"})
        , messenger (instance, debugCallback, DebugUtilsMessenger::noPerformance)
        , physicalDevice (instance, {}, {acceptWithFlag (VK_QUEUE_GRAPHICS_BIT), acceptAnything, acceptAnything, acceptAnything})
    {
        std::cout << std::endl;
    }

    virtual ~TestEnvironment () {}

    USING_PTR (TestEnvironment);
};


class TestCase final {
public:
    Device      device;
    Queue       queue;
    CommandPool commandPool;

    TestCase (const TestEnvironment& env)
        : device (env.physicalDevice, *env.physicalDevice.queueFamilies.graphics, {})
        , queue (device, *env.physicalDevice.queueFamilies.graphics)
        , commandPool (device, *env.physicalDevice.queueFamilies.graphics)
    {
    }

    virtual ~TestCase () {}

    USING_PTR (TestCase);
};


class VulkanTestEnvironment : public ::testing::Test {
protected:
    static TestEnvironment::U env;
    TestCase::U               testCase;

    PhysicalDevice& GetPhysicalDevice () { return env->physicalDevice; }
    Device&         GetDevice () { return testCase->device; }
    CommandPool&    GetCommandPool () { return testCase->commandPool; }
    Queue&          GetQueue () { return testCase->queue; }

    static void SetUpTestSuite ()
    {
        env = TestEnvironment::Create ();
    }

    static void TearDownTestSuite ()
    {
        env.reset ();
    }

    virtual void SetUp () override
    {
        testCase = TestCase::Create (*env);
    }

    virtual void TearDown () override
    {
        testCase.reset ();
    }
};

TestEnvironment::U VulkanTestEnvironment::env;

#endif