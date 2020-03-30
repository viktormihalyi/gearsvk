#ifndef SWAPCHAIN_HPP
#define SWAPCHAIN_HPP

#include "Assert.hpp"
#include "Noncopyable.hpp"
#include "PhysicalDevice.hpp"
#include "Utils.hpp"

#include <vulkan/vulkan.h>


class SwapchainSettingsProvider {
public:
    virtual VkSurfaceFormatKHR SelectSurfaceFormat (const std::vector<VkSurfaceFormatKHR>&) = 0;
    virtual VkPresentModeKHR   SelectPresentMode (const std::vector<VkPresentModeKHR>&)     = 0;
    virtual VkExtent2D         SelectExtent (const VkSurfaceCapabilitiesKHR&)               = 0;
    virtual uint32_t           SelectImageCount (const VkSurfaceCapabilitiesKHR&)           = 0;
};


class DefaultSwapchainSettings final : public SwapchainSettingsProvider {
public:
    virtual VkSurfaceFormatKHR SelectSurfaceFormat (const std::vector<VkSurfaceFormatKHR>& formats) override
    {
        for (const VkSurfaceFormatKHR& availableFormat : formats) {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                return availableFormat;
            }
        }

        throw std::runtime_error ("failed to choose swapchain surface format");
    }


    virtual VkPresentModeKHR SelectPresentMode (const std::vector<VkPresentModeKHR>& modes) override
    {
        for (VkPresentModeKHR availablePresentMode : modes) {
            if (availablePresentMode == VK_PRESENT_MODE_FIFO_KHR) {
                return availablePresentMode;
            }
        }

        throw std::runtime_error ("failed to choose swapchain present mode");
    }


    virtual VkExtent2D SelectExtent (const VkSurfaceCapabilitiesKHR& capabilities) override
    {
        if (capabilities.currentExtent.width != UINT32_MAX) {
            return capabilities.currentExtent;
        } else {
            // TODO
            VkExtent2D actualExtent = {800, 600};
            actualExtent.width      = std::clamp (capabilities.minImageExtent.width, capabilities.maxImageExtent.width, actualExtent.width);
            actualExtent.height     = std::clamp (capabilities.minImageExtent.height, capabilities.maxImageExtent.height, actualExtent.height);
            return actualExtent;
        }
    }

    virtual uint32_t SelectImageCount (const VkSurfaceCapabilitiesKHR& capabilities) override
    {
        uint32_t imageCount = capabilities.minImageCount + 1;
        if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount) {
            imageCount = capabilities.maxImageCount;
        }
        return imageCount;
    }
};


static DefaultSwapchainSettings defaultSwapchainSettings;


class Swapchain {
public:
    USING_PTR_ABSTRACT (Swapchain);

    virtual ~Swapchain () {}

    virtual VkFormat             GetImageFormat () const                                                                            = 0;
    virtual uint32_t             GetImageCount () const                                                                             = 0;
    virtual uint32_t             GetWidth () const                                                                                  = 0;
    virtual uint32_t             GetHeight () const                                                                                 = 0;
    virtual std::vector<VkImage> GetImages () const                                                                                 = 0;
    virtual uint32_t             GetNextImageIndex (VkSemaphore signalSemaphore) const                                              = 0;
    virtual void                 Present (VkQueue queue, uint32_t imageIndex, const std::vector<VkSemaphore>& waitSemaphores) const = 0;
    virtual bool                 SupportsPresenting () const                                                                        = 0;
};

class RealSwapchain : public Swapchain, public Noncopyable {
public:
    static const VkImageUsageFlags ImageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

private:
    const VkDevice device;
    VkSwapchainKHR handle;

    uint32_t           imageCount;
    VkSurfaceFormatKHR surfaceFormat;
    VkPresentModeKHR   presentMode;
    VkExtent2D         extent;

    std::vector<VkImage>        images;
    std::vector<ImageView::U>   imageViews;
    std::vector<Framebuffer::U> framebuffers;

public:
    USING_PTR (RealSwapchain);

    RealSwapchain (const PhysicalDevice& physicalDevice, VkDevice device, VkSurfaceKHR surface)
        : RealSwapchain (physicalDevice, device, surface, physicalDevice.GetQueueFamilies ())
    {
    }

    RealSwapchain (VkPhysicalDevice physicalDevice, VkDevice device, VkSurfaceKHR surface, PhysicalDevice::QueueFamilies queueFamilyIndices, SwapchainSettingsProvider& settings = defaultSwapchainSettings)
        : device (device)
    {
        VkSurfaceCapabilitiesKHR capabilities;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR (physicalDevice, surface, &capabilities);

        std::vector<VkSurfaceFormatKHR> formats;
        uint32_t                        formatCount = 0;
        vkGetPhysicalDeviceSurfaceFormatsKHR (physicalDevice, surface, &formatCount, nullptr);
        formats.resize (formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR (physicalDevice, surface, &formatCount, formats.data ());

        std::vector<VkPresentModeKHR> presentModes;
        uint32_t                      presentModeCount = 0;
        vkGetPhysicalDeviceSurfacePresentModesKHR (physicalDevice, surface, &presentModeCount, nullptr);
        presentModes.resize (presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR (physicalDevice, surface, &presentModeCount, presentModes.data ());

        surfaceFormat = settings.SelectSurfaceFormat (formats);
        presentMode   = settings.SelectPresentMode (presentModes);
        extent        = settings.SelectExtent (capabilities);
        imageCount    = settings.SelectImageCount (capabilities);

        VkSwapchainCreateInfoKHR createInfo = {};
        createInfo.sType                    = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface                  = surface;
        createInfo.minImageCount            = imageCount;
        createInfo.imageFormat              = surfaceFormat.format;
        createInfo.imageColorSpace          = surfaceFormat.colorSpace;
        createInfo.imageExtent              = extent;
        createInfo.imageArrayLayers         = 1;
        createInfo.imageUsage               = RealSwapchain::ImageUsage;

        uint32_t queueFamilyIndicesData[] = {*queueFamilyIndices.graphics, *queueFamilyIndices.presentation};
        if (queueFamilyIndices.presentation) {
            ASSERT (*queueFamilyIndices.graphics == *queueFamilyIndices.presentation);
        }

        //if (ERROR (*queueFamilyIndices.graphics != *queueFamilyIndices.presentation)) {
        createInfo.imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 1;
        createInfo.pQueueFamilyIndices   = queueFamilyIndicesData;
        //} else {
        //    createInfo.imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE;
        //    createInfo.queueFamilyIndexCount = 0;       // Optional
        //    createInfo.pQueueFamilyIndices   = nullptr; // Optional
        //}
        createInfo.preTransform   = capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode    = presentMode;
        createInfo.clipped        = VK_TRUE;
        createInfo.oldSwapchain   = VK_NULL_HANDLE;

        if (ERROR (vkCreateSwapchainKHR (device, &createInfo, nullptr, &handle) != VK_SUCCESS)) {
            throw std::runtime_error ("failed to create swapchain");
        }

        uint32_t imageCount;
        vkGetSwapchainImagesKHR (device, handle, &imageCount, nullptr);
        images.resize (imageCount);
        vkGetSwapchainImagesKHR (device, handle, &imageCount, images.data ());

        for (size_t i = 0; i < images.size (); ++i) {
            imageViews.push_back (ImageView::Create (device, images[i], GetImageFormat ()));
        }
    }

    ~RealSwapchain ()
    {
        vkDestroySwapchainKHR (device, handle, nullptr);
        handle = VK_NULL_HANDLE;
    }

    operator VkSwapchainKHR () const { return handle; }

    virtual VkFormat             GetImageFormat () const override { return surfaceFormat.format; }
    virtual uint32_t             GetImageCount () const override { return imageCount; }
    virtual uint32_t             GetWidth () const override { return extent.width; }
    virtual uint32_t             GetHeight () const override { return extent.height; }
    virtual std::vector<VkImage> GetImages () const override { return images; }

    virtual uint32_t GetNextImageIndex (VkSemaphore signalSemaphore) const override
    {
        uint32_t result;
        vkAcquireNextImageKHR (device, handle, UINT64_MAX, signalSemaphore, VK_NULL_HANDLE, &result);
        return result;
    }

    virtual bool SupportsPresenting () const { return true; }

    virtual void Present (VkQueue queue, uint32_t imageIndex, const std::vector<VkSemaphore>& waitSemaphores) const override
    {
        VkPresentInfoKHR presentInfo   = {};
        presentInfo.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = waitSemaphores.size ();
        presentInfo.pWaitSemaphores    = waitSemaphores.data ();
        presentInfo.swapchainCount     = 1;
        presentInfo.pSwapchains        = &handle;
        presentInfo.pImageIndices      = &imageIndex;
        presentInfo.pResults           = nullptr;

        vkQueuePresentKHR (queue, &presentInfo);
    }
};


class FakeSwapchain : public Swapchain {
public:
    AllocatedImage image;
    Device&        device;
    const uint32_t width;
    const uint32_t height;

    USING_PTR (FakeSwapchain);

    FakeSwapchain (Device& device, uint32_t width, uint32_t height)
        : device (device)
        , width (width)
        , height (height)
        , image (device, Image::Create (device, width, height, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, RealSwapchain::ImageUsage, 1), DeviceMemory::GPU)
    {
    }

    virtual VkFormat             GetImageFormat () const override { return image.image->GetFormat (); }
    virtual uint32_t             GetImageCount () const override { return 1; }
    virtual uint32_t             GetWidth () const override { return width; }
    virtual uint32_t             GetHeight () const override { return height; }
    virtual std::vector<VkImage> GetImages () const override { return {*image.image}; }

    virtual uint32_t GetNextImageIndex (VkSemaphore signalSemaphore) const override
    {
        ASSERT (signalSemaphore == VK_NULL_HANDLE);
        return 0;
    }

    virtual bool SupportsPresenting () const { return false; }

    virtual void Present (VkQueue queue, uint32_t imageIndex, const std::vector<VkSemaphore>& waitSemaphores) const override
    {
        throw std::runtime_error ("fake swapchain cannot present");
    }
};


#endif