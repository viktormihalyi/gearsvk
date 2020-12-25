#ifndef VULKANTESTENVIRONMENT_HPP
#define VULKANTESTENVIRONMENT_HPP

#include "GearsVkAPI.hpp"

#include "Allocator.hpp"
#include "DeviceExtra.hpp"
#include "Ptr.hpp"
#include "VulkanUtils.hpp"
#include "VulkanWrapper.hpp"
#include "Window.hpp"


USING_PTR (Presentable);
class GEARSVK_API Presentable : public SwapchainProvider {
    USING_CREATE (Presentable);

private:
    SurfaceU   surface;
    SwapchainU swapchain;

public:
    Presentable ();
    Presentable (const PhysicalDevice& physicalDevice, VkDevice device, SurfaceU&& surface);
    Presentable (SwapchainU&& swapchain);

    void operator= (Presentable&&) noexcept;

    void Clear ();

    virtual Swapchain& GetSwapchain () override { return *swapchain; }
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
    AllocatorU           alloactor;

    VulkanEnvironment (std::optional<DebugUtilsMessenger::Callback> callback = testDebugCallback);

    virtual ~VulkanEnvironment ();

    void Wait () const;

    PresentableP CreatePresentable (SurfaceU&& surface) const;
    PresentableP CreatePresentable (Window& window) const;
};


#endif