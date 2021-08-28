#ifndef VULKANTESTENVIRONMENT_HPP
#define VULKANTESTENVIRONMENT_HPP

#include "RenderGraph/RenderGraphAPI.hpp"

#include "VulkanWrapper/DebugUtilsMessenger.hpp"
#include "VulkanWrapper/Swapchain.hpp"
#include "VulkanWrapper/Surface.hpp"

#include <memory>

#include <optional>

namespace GVK {
class Instance;
class PhysicalDevice;
class Device;
class Queue;
class CommandPool;
class DeviceExtra;
class Allocator;
class Surface;
} // namespace GVK

namespace RG {

class Window;
class VulkanEnvironment;

class GVK_RENDERER_API Presentable : public GVK::SwapchainProvider {
private:
    std::unique_ptr<Window>    window;
    std::unique_ptr<GVK::Surface>   surface;
    std::unique_ptr<GVK::Swapchain> swapchain;

public:
    Presentable (VulkanEnvironment& env, std::unique_ptr<GVK::Surface>&& surface, std::unique_ptr<GVK::SwapchainSettingsProvider>&& settingsProvider);
    Presentable (VulkanEnvironment& env, Window& window, std::unique_ptr<GVK::SwapchainSettingsProvider>&& settingsProvider);
    Presentable (VulkanEnvironment& env, std::unique_ptr<Window>&& window, std::unique_ptr<GVK::SwapchainSettingsProvider>&& settingsProvider);

    virtual GVK::Swapchain& GetSwapchain () override;

    const GVK::Surface& GetSurface () const;
    Window& GetWindow ();

    std::optional<double> GetRefreshRate () const;
};


GVK_RENDERER_API
void defaultDebugCallback (VkDebugUtilsMessageSeverityFlagBitsEXT,
                        VkDebugUtilsMessageTypeFlagsEXT,
                        const VkDebugUtilsMessengerCallbackDataEXT* callbackData);

class GVK_RENDERER_API VulkanEnvironment {
public:
    std::unique_ptr<GVK::Instance>            instance;
    std::unique_ptr<GVK::DebugUtilsMessenger> messenger;
    std::unique_ptr<GVK::PhysicalDevice>      physicalDevice;
    std::unique_ptr<GVK::Device>              device;
    std::unique_ptr<GVK::Queue>               graphicsQueue;
    std::unique_ptr<GVK::Queue>               presentQueue;
    std::unique_ptr<GVK::CommandPool>         commandPool;
    std::unique_ptr<GVK::DeviceExtra>         deviceExtra;
    std::unique_ptr<GVK::Allocator>           allocator;

    VulkanEnvironment (std::optional<GVK::DebugUtilsMessenger::Callback> callback           = defaultDebugCallback,
                       const std::vector<const char*>&              instanceExtensions = {},
                       const std::vector<const char*>&              deviceExtensions = {});

    virtual ~VulkanEnvironment ();

    void Wait () const;

    bool CheckForPhsyicalDeviceSupport (const Presentable&);
};

} // namespace RG


#endif