#ifndef SWAPCHAIN_HPP
#define SWAPCHAIN_HPP

#include "GearsVkAPI.hpp"

#include "Assert.hpp"
#include "Framebuffer.hpp"
#include "ImageView.hpp"
#include "Noncopyable.hpp"
#include "PhysicalDevice.hpp"
#include "Utils.hpp"

#include <vulkan/vulkan.h>


class GEARSVK_API SwapchainSettingsProvider {
public:
    virtual VkSurfaceFormatKHR SelectSurfaceFormat (const std::vector<VkSurfaceFormatKHR>&) = 0;
    virtual VkPresentModeKHR   SelectPresentMode (const std::vector<VkPresentModeKHR>&)     = 0;
    virtual VkExtent2D         SelectExtent (const VkSurfaceCapabilitiesKHR&)               = 0;
    virtual uint32_t           SelectImageCount (const VkSurfaceCapabilitiesKHR&)           = 0;
};


class GEARSVK_API DefaultSwapchainSettings final : public SwapchainSettingsProvider {
public:
    virtual VkSurfaceFormatKHR SelectSurfaceFormat (const std::vector<VkSurfaceFormatKHR>& formats) override;
    virtual VkPresentModeKHR   SelectPresentMode (const std::vector<VkPresentModeKHR>& modes) override;
    virtual VkExtent2D         SelectExtent (const VkSurfaceCapabilitiesKHR& capabilities) override;
    virtual uint32_t           SelectImageCount (const VkSurfaceCapabilitiesKHR& capabilities) override;
};

GEARSVK_API
extern DefaultSwapchainSettings defaultSwapchainSettings;


USING_PTR (Swapchain);
class GEARSVK_API Swapchain {
public:
    virtual ~Swapchain () = default;

    virtual VkFormat                         GetImageFormat () const = 0;
    virtual uint32_t                         GetImageCount () const  = 0;
    virtual uint32_t                         GetWidth () const       = 0;
    virtual uint32_t                         GetHeight () const      = 0;
    virtual std::vector<VkImage>             GetImages () const      = 0;
    virtual const std::vector<ImageView2DU>& GetImageViews () const  = 0;

    virtual uint32_t GetNextImageIndex (VkSemaphore signalSemaphore) const                                              = 0;
    virtual void     Present (VkQueue queue, uint32_t imageIndex, const std::vector<VkSemaphore>& waitSemaphores) const = 0;
    virtual bool     SupportsPresenting () const                                                                        = 0;
    virtual void     Recreate ()                                                                                        = 0;
    virtual void     RecreateForSurface (VkSurfaceKHR surface)                                                          = 0;
};

class OutOfDateSwapchain : public std::runtime_error {
public:
    OutOfDateSwapchain ()
        : std::runtime_error ("out of date swapchain detected")
    {
    }
};

USING_PTR (RealSwapchain);
class GEARSVK_API RealSwapchain : public Swapchain,
                                  public Noncopyable {
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
        VkSwapchainKHR handle;

        uint32_t           imageCount;
        VkSurfaceFormatKHR surfaceFormat;
        VkPresentModeKHR   presentMode;
        VkExtent2D         extent;

        std::vector<VkImage>      images;
        std::vector<ImageView2DU> imageViews;
        std::vector<FramebufferU> framebuffers;

        CreateResult ()
            : handle (VK_NULL_HANDLE)
        {
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
    USING_CREATE (RealSwapchain);

    RealSwapchain (const PhysicalDevice& physicalDevice, VkDevice device, VkSurfaceKHR surface);
    RealSwapchain (VkPhysicalDevice physicalDevice, VkDevice device, VkSurfaceKHR surface, PhysicalDevice::QueueFamilies queueFamilyIndices, SwapchainSettingsProvider& settings = defaultSwapchainSettings);

    virtual void Recreate () override;

    virtual void RecreateForSurface (VkSurfaceKHR surface) override;

    ~RealSwapchain ();

    operator VkSwapchainKHR () const { return createResult.handle; }

    virtual VkFormat                         GetImageFormat () const override { return createResult.surfaceFormat.format; }
    virtual uint32_t                         GetImageCount () const override { return createResult.imageCount; }
    virtual uint32_t                         GetWidth () const override { return createResult.extent.width; }
    virtual uint32_t                         GetHeight () const override { return createResult.extent.height; }
    virtual std::vector<VkImage>             GetImages () const override { return createResult.images; }
    virtual const std::vector<ImageView2DU>& GetImageViews () const override { return createResult.imageViews; }


    virtual uint32_t GetNextImageIndex (VkSemaphore signalSemaphore) const override;

    virtual bool SupportsPresenting () const { return true; }

    virtual void Present (VkQueue queue, uint32_t imageIndex, const std::vector<VkSemaphore>& waitSemaphores) const override;
};


USING_PTR (FakeSwapchain);
class GEARSVK_API FakeSwapchain : public Swapchain {
private:
    AllocatedImage image;
    Device&        device;
    const uint32_t width;
    const uint32_t height;

public:
    USING_CREATE (FakeSwapchain);

    FakeSwapchain (Device& device, VkQueue queue, VkCommandPool commandPool, uint32_t width, uint32_t height);

    virtual VkFormat             GetImageFormat () const override { return image.image->GetFormat (); }
    virtual uint32_t             GetImageCount () const override { return 1; }
    virtual uint32_t             GetWidth () const override { return width; }
    virtual uint32_t             GetHeight () const override { return height; }
    virtual std::vector<VkImage> GetImages () const override { return {*image.image}; }
    virtual void                 Recreate () override {}
    virtual void                 RecreateForSurface (VkSurfaceKHR) override {}

    virtual uint32_t GetNextImageIndex (VkSemaphore signalSemaphore) const override
    {
        ASSERT (signalSemaphore == VK_NULL_HANDLE);
        return 0;
    }

    virtual const std::vector<ImageView2DU>& GetImageViews () const { throw std::runtime_error ("no imageview for fake swapchain"); }

    virtual bool SupportsPresenting () const { return false; }

    virtual void Present (VkQueue, uint32_t, const std::vector<VkSemaphore>&) const override
    {
        throw std::runtime_error ("fake swapchain cannot present");
    }
};


#endif