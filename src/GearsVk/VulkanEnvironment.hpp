#ifndef VULKANTESTENVIRONMENT_HPP
#define VULKANTESTENVIRONMENT_HPP

#include "GearsVkAPI.hpp"

#include "DeviceExtra.hpp"
#include "Ptr.hpp"
#include "VulkanUtils.hpp"
#include "VulkanWrapper.hpp"
#include "Window.hpp"


USING_PTR (VulkanEnvironment);
class GEARSVK_API IVulkanEnvironment {
public:
    virtual ~IVulkanEnvironment () = default;

    virtual Device&      GetDevice () const            = 0;
    virtual CommandPool& GetCommandPool () const       = 0;
    virtual Queue&       GetGraphicsQueue () const     = 0;
    virtual Queue&       GetPresentationQueue () const = 0;
};

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

    // surface and swapchain are created if a window is provided in the ctor
    SurfaceU   surface;
    SwapchainU swapchain;

    std::vector<VulkanObjectW> objects;

    VulkanEnvironment (std::optional<WindowRef> window = std::nullopt, std::optional<DebugUtilsMessenger::Callback> callback = testDebugCallback);

    virtual ~VulkanEnvironment ();

    void Wait () const;

    VulkanObjectP FindObject (const GearsVk::UUID& uuid);

    void RegisterObject (const VulkanObjectP& obj);
};


#endif