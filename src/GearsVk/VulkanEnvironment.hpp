#ifndef VULKANTESTENVIRONMENT_HPP
#define VULKANTESTENVIRONMENT_HPP

#include "GearsVkAPI.hpp"

#include "DeviceExtra.hpp"
#include "GLFWWindow.hpp"
#include "Ptr.hpp"
#include "TerminalColors.hpp"
#include "VulkanUtils.hpp"
#include "VulkanWrapper.hpp"

GEARSVK_API
void testDebugCallback (VkDebugUtilsMessageSeverityFlagBitsEXT,
                        VkDebugUtilsMessageTypeFlagsEXT,
                        const VkDebugUtilsMessengerCallbackDataEXT* callbackData);


class GEARSVK_API VulkanEnvironment {
public:
    Instance::U            instance;
    DebugUtilsMessenger::U messenger;
    PhysicalDevice::U      physicalDevice;
    Device::U              device;
    Queue::U               graphicsQueue;
    Queue::U               presentQueue;
    CommandPool::U         commandPool;
    DeviceExtra::U         deviceExtra;

    // surface and swapchain are created if a window is provided in the ctor
    Surface::U   surface;
    Swapchain::U swapchain;


    USING_PTR (VulkanEnvironment);

    VulkanEnvironment (std::optional<Window::Ref> window = std::nullopt, std::optional<DebugUtilsMessenger::Callback> callback = testDebugCallback);

    virtual ~VulkanEnvironment ();

    void Wait () const;
};


#endif