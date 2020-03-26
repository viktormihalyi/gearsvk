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


class Swapchain : public Noncopyable {
private:
    static const VkImageUsageFlags ImageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

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
    USING_PTR (Swapchain);

    Swapchain (const PhysicalDevice& physicalDevice, VkDevice device, VkSurfaceKHR surface)
        : Swapchain (physicalDevice, device, surface, physicalDevice.GetQueueFamilies ())
    {
    }

    Swapchain (VkPhysicalDevice physicalDevice, VkDevice device, VkSurfaceKHR surface, PhysicalDevice::QueueFamilies queueFamilyIndices, SwapchainSettingsProvider& settings = defaultSwapchainSettings)
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
        createInfo.imageUsage               = Swapchain::ImageUsage;

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

    ~Swapchain ()
    {
        vkDestroySwapchainKHR (device, handle, nullptr);
        handle = VK_NULL_HANDLE;
    }

    operator VkSwapchainKHR () const { return handle; }

    VkFormat             GetImageFormat () const { return surfaceFormat.format; }
    uint32_t             GetImageCount () const { return imageCount; }
    uint32_t             GetWidth () const { return extent.width; }
    uint32_t             GetHeight () const { return extent.height; }
    std::vector<VkImage> GetImages () const { return images; }
};


#endif