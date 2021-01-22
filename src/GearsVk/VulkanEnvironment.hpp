#ifndef VULKANTESTENVIRONMENT_HPP
#define VULKANTESTENVIRONMENT_HPP

#include "GearsVkAPI.hpp"

#include "DebugUtilsMessenger.hpp"
#include "Swapchain.hpp"

#include "Ptr.hpp"

#include <optional>


class Instance;
class Window;
class PhysicalDevice;
class Device;
class Queue;
class CommandPool;
class DeviceExtra;
class Allocator;
class Surface;

class VulkanEnvironment;

USING_PTR (Presentable);
class GEARSVK_API Presentable : public SwapchainProvider {
    USING_CREATE (Presentable);

private:
    U<Window>    window;
    U<Surface>   surface;
    U<Swapchain> swapchain;

public:
    Presentable (VulkanEnvironment& env, U<Surface>&& surface, SwapchainSettingsProvider& settingsProvider = defaultSwapchainSettings);
    Presentable (VulkanEnvironment& env, Window& window, SwapchainSettingsProvider& settingsProvider = defaultSwapchainSettings);
    Presentable (VulkanEnvironment& env, U<Window>&& window, SwapchainSettingsProvider& settingsProvider = defaultSwapchainSettings);

    virtual Swapchain& GetSwapchain () override;

    const Surface& GetSurface () const;
};


GEARSVK_API
void testDebugCallback (VkDebugUtilsMessageSeverityFlagBitsEXT,
                        VkDebugUtilsMessageTypeFlagsEXT,
                        const VkDebugUtilsMessengerCallbackDataEXT* callbackData);

USING_PTR (VulkanEnvironment);
class GEARSVK_API VulkanEnvironment {
    USING_CREATE (VulkanEnvironment)

public:
    U<Instance>            instance;
    U<DebugUtilsMessenger> messenger;
    U<PhysicalDevice>      physicalDevice;
    U<Device>              device;
    U<Queue>               graphicsQueue;
    U<Queue>               presentQueue;
    U<CommandPool>         commandPool;
    U<DeviceExtra>         deviceExtra;
    U<Allocator>           alloactor;

    VulkanEnvironment (std::optional<DebugUtilsMessenger::Callback> callback = testDebugCallback);

    virtual ~VulkanEnvironment ();

    void Wait () const;

    void RecreateForPresentable (const Presentable&);
};


#endif