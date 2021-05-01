#ifndef SWAPCHAIN_HPP
#define SWAPCHAIN_HPP

#include "GearsVkAPI.hpp"

#include "Assert.hpp"
#include "Framebuffer.hpp"
#include "ImageView.hpp"
#include "MovablePtr.hpp"
#include "PhysicalDevice.hpp"
#include "Utils.hpp"

#include <vulkan/vulkan.h>

namespace GVK {

class GVK_RENDERER_API SwapchainSettingsProvider {
public:
    virtual VkSurfaceFormatKHR SelectSurfaceFormat (const std::vector<VkSurfaceFormatKHR>&) = 0;
    virtual VkPresentModeKHR   SelectPresentMode (const std::vector<VkPresentModeKHR>&)     = 0;
    virtual VkExtent2D         SelectExtent (const VkSurfaceCapabilitiesKHR&)               = 0;
    virtual uint32_t           SelectImageCount (const VkSurfaceCapabilitiesKHR&)           = 0;
};


class GVK_RENDERER_API DefaultSwapchainSettings : public SwapchainSettingsProvider {
public:
    virtual VkSurfaceFormatKHR SelectSurfaceFormat (const std::vector<VkSurfaceFormatKHR>& formats) override;
    virtual VkPresentModeKHR   SelectPresentMode (const std::vector<VkPresentModeKHR>& modes) override;
    virtual VkExtent2D         SelectExtent (const VkSurfaceCapabilitiesKHR& capabilities) override;
    virtual uint32_t           SelectImageCount (const VkSurfaceCapabilitiesKHR& capabilities) override;
};

class GVK_RENDERER_API DefaultSwapchainSettingsSingleImage : public DefaultSwapchainSettings {
public:
    virtual uint32_t SelectImageCount (const VkSurfaceCapabilitiesKHR& capabilities) override;
};

GVK_RENDERER_API
extern DefaultSwapchainSettings defaultSwapchainSettings;

GVK_RENDERER_API
extern DefaultSwapchainSettingsSingleImage defaultSwapchainSettingsSingleImage;


class GVK_RENDERER_API Swapchain {
public:
    virtual ~Swapchain () = default;

    virtual VkFormat                                         GetImageFormat () const  = 0;
    virtual uint32_t                                         GetImageCount () const   = 0;
    virtual uint32_t                                         GetWidth () const        = 0;
    virtual uint32_t                                         GetHeight () const       = 0;
    virtual std::vector<VkImage>                             GetImages () const       = 0;
    virtual std::vector<std::unique_ptr<InheritedImage>>     GetImageObjects () const = 0;
    virtual const std::vector<std::unique_ptr<ImageView2D>>& GetImageViews () const   = 0;

    virtual uint32_t GetNextImageIndex (VkSemaphore signalSemaphore, VkFence fence = VK_NULL_HANDLE) const              = 0;
    virtual void     Present (VkQueue queue, uint32_t imageIndex, const std::vector<VkSemaphore>& waitSemaphores) const = 0;
    virtual bool     SupportsPresenting () const                                                                        = 0;
    virtual void     Recreate ()                                                                                        = 0;
    virtual void     RecreateForSurface (VkSurfaceKHR surface)                                                          = 0;
    virtual bool     IsEqualSettings (const Swapchain& other)                                                           = 0;
};

class GVK_RENDERER_API OutOfDateSwapchain : public std::runtime_error {
public:
    const Swapchain& swapchain;
    OutOfDateSwapchain (const Swapchain& swapchain)
        : std::runtime_error { "out of date swapchain detected" }
        , swapchain { swapchain }
    {
    }
};

class GVK_RENDERER_API RealSwapchain : public Swapchain {
public:
    static const VkImageUsageFlags ImageUsage;

private:
    struct CreateSettings {
        VkPhysicalDevice              physicalDevice;
        VkDevice                      device;
        VkSurfaceKHR                  surface;
        PhysicalDevice::QueueFamilies queueFamilyIndices;
        SwapchainSettingsProvider&    settings;
    };

    struct CreateResult {
        GVK::MovablePtr<VkSwapchainKHR> handle;

        uint32_t           imageCount;
        VkSurfaceFormatKHR surfaceFormat;
        VkPresentModeKHR   presentMode;
        VkExtent2D         extent;

        std::vector<VkImage>                      images;
        std::vector<std::unique_ptr<ImageView2D>> imageViews;
        std::vector<std::unique_ptr<Framebuffer>> framebuffers;

        CreateResult ()
            : handle (VK_NULL_HANDLE)
            , imageCount (0)
        {
        }

        bool IsEqualSettings (const CreateResult& other)
        {
            return imageCount == other.imageCount &&
                   surfaceFormat.format == other.surfaceFormat.format &&
                   surfaceFormat.colorSpace == other.surfaceFormat.colorSpace &&
                   presentMode == other.presentMode &&
                   extent.width == other.extent.width &&
                   extent.height == other.extent.height;
        }

        void Clear ()
        {
            handle     = VK_NULL_HANDLE;
            imageCount = 0;
            images.clear ();
            imageViews.clear ();
            framebuffers.clear ();
        }
    };

    CreateSettings createSettings;
    CreateResult   createResult;

    static CreateResult CreateForResult (const CreateSettings& createSettings);

public:
    RealSwapchain (const PhysicalDevice& physicalDevice, VkDevice device, VkSurfaceKHR surface, SwapchainSettingsProvider& settings = defaultSwapchainSettings);
    RealSwapchain (VkPhysicalDevice physicalDevice, VkDevice device, VkSurfaceKHR surface, PhysicalDevice::QueueFamilies queueFamilyIndices, SwapchainSettingsProvider& settings = defaultSwapchainSettings);

    virtual void Recreate () override;

    virtual void RecreateForSurface (VkSurfaceKHR surface) override;

    virtual bool IsEqualSettings (const Swapchain& other) override;

    virtual ~RealSwapchain ();

    operator VkSwapchainKHR () const { return createResult.handle; }

    virtual VkFormat                                         GetImageFormat () const override { return createResult.surfaceFormat.format; }
    virtual uint32_t                                         GetImageCount () const override { return createResult.imageCount; }
    virtual uint32_t                                         GetWidth () const override { return createResult.extent.width; }
    virtual uint32_t                                         GetHeight () const override { return createResult.extent.height; }
    virtual std::vector<VkImage>                             GetImages () const override { return createResult.images; }
    virtual const std::vector<std::unique_ptr<ImageView2D>>& GetImageViews () const override { return createResult.imageViews; }
    virtual std::vector<std::unique_ptr<InheritedImage>>     GetImageObjects () const override;

    virtual uint32_t GetNextImageIndex (VkSemaphore signalSemaphore, VkFence fenceToSignal = VK_NULL_HANDLE) const override;

    virtual bool SupportsPresenting () const override { return true; }

    virtual void Present (VkQueue queue, uint32_t imageIndex, const std::vector<VkSemaphore>& waitSemaphores) const override;
};


class GVK_RENDERER_API ExtentProvider {
public:
    virtual ~ExtentProvider () = default;

    virtual std::pair<uint32_t, uint32_t> GetExtent () = 0;
};


class GVK_RENDERER_API SwapchainProvider : public ExtentProvider {
public:
    virtual ~SwapchainProvider () = default;

    virtual Swapchain& GetSwapchain () = 0;

private:
    virtual std::pair<uint32_t, uint32_t> GetExtent () override final
    {
        return { GetSwapchain ().GetWidth (), GetSwapchain ().GetHeight () };
    }
};


class GVK_RENDERER_API FakeSwapchain : public Swapchain {
private:
    std::unique_ptr<Image>                    image;
    std::vector<std::unique_ptr<ImageView2D>> imageViews;
    const DeviceExtra&                        device;
    const uint32_t                            width;
    const uint32_t                            height;

public:
    FakeSwapchain (const DeviceExtra& device, uint32_t width, uint32_t height);

    virtual VkFormat             GetImageFormat () const override { return image->GetFormat (); }
    virtual uint32_t             GetImageCount () const override { return 1; }
    virtual uint32_t             GetWidth () const override { return width; }
    virtual uint32_t             GetHeight () const override { return height; }
    virtual std::vector<VkImage> GetImages () const override { return { *image }; }
    virtual void                 Recreate () override {}
    virtual void                 RecreateForSurface (VkSurfaceKHR) override {}
    virtual bool                 IsEqualSettings (const Swapchain&) override { return true; }

    virtual uint32_t GetNextImageIndex (VkSemaphore signalSemaphore, VkFence fenceToSignal = VK_NULL_HANDLE) const override
    {
        GVK_ASSERT (signalSemaphore == VK_NULL_HANDLE);
        GVK_ASSERT (fenceToSignal == VK_NULL_HANDLE);
        return 0;
    }

    virtual const std::vector<std::unique_ptr<ImageView2D>>& GetImageViews () const override { return imageViews; }

    virtual bool SupportsPresenting () const override { return false; }

    virtual void Present (VkQueue, uint32_t, const std::vector<VkSemaphore>&) const override
    {
        throw std::runtime_error ("fake swapchain cannot present");
    }
};

} // namespace GVK

#endif