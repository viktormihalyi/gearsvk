#ifndef VULKANTESTENVIRONMENT_HPP
#define VULKANTESTENVIRONMENT_HPP

#include "RenderGraph/RenderGraphAPI.hpp"

#include "VulkanWrapper/DebugReportCallback.hpp"
#include "VulkanWrapper/DebugUtilsMessenger.hpp"
#include "VulkanWrapper/Swapchain.hpp"
#include "VulkanWrapper/Surface.hpp"

#include <memory>

#include <optional>

namespace GVK {


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

class GVK_RENDERER_API Presentable : public SwapchainProvider {
private:
    std::unique_ptr<Window>    window;
    std::unique_ptr<Surface>   surface;
    std::unique_ptr<Swapchain> swapchain;

public:
    Presentable (VulkanEnvironment& env, std::unique_ptr<Surface>&& surface, SwapchainSettingsProvider& settingsProvider = defaultSwapchainSettings);
    Presentable (VulkanEnvironment& env, Window& window, SwapchainSettingsProvider& settingsProvider = defaultSwapchainSettings);
    Presentable (VulkanEnvironment& env, std::unique_ptr<Window>&& window, SwapchainSettingsProvider& settingsProvider = defaultSwapchainSettings);

    virtual Swapchain& GetSwapchain () override;

    const Surface& GetSurface () const;
    Window& GetWindow ();

    std::optional<double> GetRefreshRate () const;
};


GVK_RENDERER_API
void testDebugCallback (VkDebugUtilsMessageSeverityFlagBitsEXT,
                        VkDebugUtilsMessageTypeFlagsEXT,
                        const VkDebugUtilsMessengerCallbackDataEXT* callbackData);

class GVK_RENDERER_API VulkanEnvironment {
public:
    std::unique_ptr<Instance>            instance;
    std::unique_ptr<DebugUtilsMessenger> messenger;
    std::unique_ptr<DebugReportCallback> debugReportCallback;
    std::unique_ptr<PhysicalDevice>      physicalDevice;
    std::unique_ptr<Device>              device;
    std::unique_ptr<Queue>               graphicsQueue;
    std::unique_ptr<Queue>               presentQueue;
    std::unique_ptr<CommandPool>         commandPool;
    std::unique_ptr<DeviceExtra>         deviceExtra;
    std::unique_ptr<Allocator>           allocator;

    VulkanEnvironment (std::optional<DebugUtilsMessenger::Callback> callback = testDebugCallback);

    virtual ~VulkanEnvironment ();

    void Wait () const;

    void RecreateForPresentable (const Presentable&);
};

} // namespace GVK


#endif