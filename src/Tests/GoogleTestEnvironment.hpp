#ifndef GOOGLETESTENVIRONMENT_HPP
#define GOOGLETESTENVIRONMENT_HPP

#include "Ptr.hpp"

// from gtest
#include "gtest/gtest.h"

// from std
#include <filesystem>
#include <optional>
#include <string>

// from vulkan
#include <vulkan/vulkan.h>

namespace GVK {
class VulkanEnvironment;
class ImageData;
class Presentable;
class PhysicalDevice;
class Swapchain;
class Device;
class CommandPool;
class Queue;
class Window;
class DeviceExtra;
class Image;
} // namespace GVK


extern const std::filesystem::path ReferenceImagesFolder;
extern const std::filesystem::path TempFolder;


void gtestDebugCallback (VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
                         VkDebugUtilsMessageTypeFlagsEXT             messageType,
                         const VkDebugUtilsMessengerCallbackDataEXT* callbackData);


using EmptyTestEnvironment = ::testing::Test;


class GoogleTestEnvironmentBase : public ::testing::Test {
protected:
    U<GVK::VulkanEnvironment> env;
    U<GVK::Window>            window;
    Ptr<GVK::Presentable>     presentable;

    GVK::PhysicalDevice& GetPhysicalDevice ();
    GVK::Device&         GetDevice ();
    GVK::CommandPool&    GetCommandPool ();
    GVK::Queue&          GetGraphicsQueue ();
    GVK::DeviceExtra&    GetDeviceExtra ();
    GVK::Window&         GetWindow ();
    GVK::Swapchain&      GetSwapchain ();

    virtual ~GoogleTestEnvironmentBase () override = default;

    virtual void SetUp ()    = 0;
    virtual void TearDown () = 0;

    void CompareImages (const std::string& imageName, const GVK::Image& image, std::optional<VkImageLayout> transitionFrom = std::nullopt);

    void CompareImages (const std::string& name, const GVK::ImageData& actualImage);
};


// no window, swapchain, surface
class HeadlessGoogleTestEnvironment : public GoogleTestEnvironmentBase {
protected:
    virtual void SetUp () override;
    virtual void TearDown () override;
};

// window shown by default
class ShownWindowGoogleTestEnvironment : public GoogleTestEnvironmentBase {
protected:
    virtual void SetUp () override;
    virtual void TearDown () override;
};


// window hidden by default
class HiddenWindowGoogleTestEnvironment : public GoogleTestEnvironmentBase {
protected:
    virtual void SetUp () override;
    virtual void TearDown () override;
};


#endif