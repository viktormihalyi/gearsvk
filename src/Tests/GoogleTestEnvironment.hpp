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


extern const std::filesystem::path ReferenceImagesFolder;
extern const std::filesystem::path TempFolder;


void gtestDebugCallback (VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
                         VkDebugUtilsMessageTypeFlagsEXT             messageType,
                         const VkDebugUtilsMessengerCallbackDataEXT* callbackData);


using EmptyTestEnvironment = ::testing::Test;


class GoogleTestEnvironmentBase : public ::testing::Test {
protected:
    U<VulkanEnvironment> env;
    U<Window>            window;
    Ptr<Presentable>     presentable;

    PhysicalDevice& GetPhysicalDevice ();
    Device&         GetDevice ();
    CommandPool&    GetCommandPool ();
    Queue&          GetGraphicsQueue ();
    DeviceExtra&    GetDeviceExtra ();
    Window&         GetWindow ();
    Swapchain&      GetSwapchain ();

    virtual ~GoogleTestEnvironmentBase () override = default;

    virtual void SetUp ()    = 0;
    virtual void TearDown () = 0;

    void CompareImages (const std::string& imageName, const Image& image, std::optional<VkImageLayout> transitionFrom = std::nullopt);

    void CompareImages (const std::string& name, const ImageData& actualImage);
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