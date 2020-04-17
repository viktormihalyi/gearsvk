#ifndef VULKANTESTENVIRONMENT_HPP
#define VULKANTESTENVIRONMENT_HPP

#include "GLFWWindow.hpp"
#include "Ptr.hpp"
#include "TerminalColors.hpp"
#include "VulkanUtils.hpp"
#include "VulkanWrapper.hpp"

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

    TestEnvironment (std::vector<const char*> instanceExtensions, std::optional<Window::Ref> window = std::nullopt, DebugUtilsMessenger::Callback callback = testDebugCallback);

    void Wait () const;
};


#endif