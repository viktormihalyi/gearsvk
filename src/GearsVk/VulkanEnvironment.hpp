#ifndef VULKANTESTENVIRONMENT_HPP
#define VULKANTESTENVIRONMENT_HPP

#include "GearsVkAPI.hpp"

#include "Allocator.hpp"
#include "DebugUtilsMessenger.hpp"
#include "DeviceExtra.hpp"
#include "Ptr.hpp"
#include "VulkanUtils.hpp"
#include "VulkanWrapper.hpp"
#include "Window.hpp"

#include <optional>

class VulkanEnvironment;

USING_PTR (Presentable);
class GEARSVK_API Presentable : public SwapchainProvider {
    USING_CREATE (Presentable);

private:
    SurfaceU   surface;
    SwapchainU swapchain;

public:
    Presentable (VulkanEnvironment& env, SurfaceU&& surface, SwapchainSettingsProvider& settingsProvider = defaultSwapchainSettings);
    Presentable (VulkanEnvironment& env, Window& window, SwapchainSettingsProvider& settingsProvider = defaultSwapchainSettings);

    virtual Swapchain& GetSwapchain () override { return *swapchain; }

    const Surface& GetSurface () const { return *surface; }
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

    void RecreateForPresentable (const Presentable&);
};


#endif