#ifndef VULKANTESTENVIRONMENT_HPP
#define VULKANTESTENVIRONMENT_HPP

#include "GearsVkAPI.hpp"

#include "Allocator.hpp"
#include "DeviceExtra.hpp"
#include "Ptr.hpp"
#include "VulkanUtils.hpp"
#include "VulkanWrapper.hpp"
#include "Window.hpp"


GEARSVK_API
void testDebugCallback (VkDebugUtilsMessageSeverityFlagBitsEXT,
                        VkDebugUtilsMessageTypeFlagsEXT,
                        const VkDebugUtilsMessengerCallbackDataEXT* callbackData);

USING_PTR (VulkanEnvironment);
class GEARSVK_API VulkanEnvironment {
    USING_CREATE (VulkanEnvironment)
public:
    InstanceU            instance;
    DebugUtilsMessengerU messenger;
    PhysicalDeviceU      physicalDevice;
    DeviceU              device;
    QueueU               graphicsQueue;
    QueueU               presentQueue;
    CommandPoolU         commandPool;
    DeviceExtraU         deviceExtra;
    AllocatorU           alloactor;

    // surface and swapchain are created if a window is provided in the ctor
    SurfaceU   surface;
    SwapchainU swapchain;

    VulkanEnvironment (std::optional<WindowRef> window = std::nullopt, std::optional<DebugUtilsMessenger::Callback> callback = testDebugCallback);

    virtual ~VulkanEnvironment ();

    void Wait () const;
};


#endif