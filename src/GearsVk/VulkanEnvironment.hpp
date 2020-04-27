#ifndef VULKANTESTENVIRONMENT_HPP
#define VULKANTESTENVIRONMENT_HPP

#include "DeviceExtra.hpp"
#include "GLFWWindow.hpp"
#include "Ptr.hpp"
#include "TerminalColors.hpp"
#include "VulkanUtils.hpp"
#include "VulkanWrapper.hpp"


void testDebugCallback (VkDebugUtilsMessageSeverityFlagBitsEXT,
                        VkDebugUtilsMessageTypeFlagsEXT,
                        const VkDebugUtilsMessengerCallbackDataEXT* callbackData);


class VulkanEnvironment {
public:
    Instance::U            instance;
    DebugUtilsMessenger::U messenger;
    PhysicalDevice::U      physicalDevice;
    Device::U              device;
    Queue::U               graphicsQueue;
    CommandPool::U         commandPool;
    DeviceExtra::U         deviceExtra;

    // surface and swapchain are created if a window is provided in the ctor
    Surface::U   surface;
    Swapchain::U swapchain;

    enum class Mode {
        Debug,
        Release,
    };

    USING_PTR (VulkanEnvironment);

    VulkanEnvironment (Mode mode, std::optional<Window::Ref> window = std::nullopt, std::optional<DebugUtilsMessenger::Callback> callback = testDebugCallback);

    virtual ~VulkanEnvironment ();

    void Wait () const;

    static VulkanEnvironment::U CreateForBuildType (std::optional<Window::Ref> window = std::nullopt, std::optional<DebugUtilsMessenger::Callback> callback = testDebugCallback);
};


class DebugVulkanEnvironment : public VulkanEnvironment {
public:
    USING_PTR (DebugVulkanEnvironment);
    DebugVulkanEnvironment (std::optional<Window::Ref> window = std::nullopt, std::optional<DebugUtilsMessenger::Callback> callback = testDebugCallback)
        : VulkanEnvironment (Mode::Debug, window, callback)
    {
    }
};


class ReleaseVulkanEnvironment : public VulkanEnvironment {
public:
    USING_PTR (ReleaseVulkanEnvironment);
    ReleaseVulkanEnvironment (std::optional<Window::Ref> window = std::nullopt)
        : VulkanEnvironment (Mode::Release, window, std::nullopt)
    {
    }
};


#endif