#ifndef SWAPCHAIN_HPP
#define SWAPCHAIN_HPP

#include "Assert.hpp"
#include "Noncopyable.hpp"
#include "PhysicalDevice.hpp"
#include "Utils.hpp"

#include <vulkan/vulkan.h>

class Swapchain : public Noncopyable {
private:
    const VkPhysicalDevice physicalDevice;
    const VkDevice         device;
    VkSwapchainKHR         handle;

    uint32_t imageCount;

public:
    VkSurfaceFormatKHR surfaceFormat;
    VkPresentModeKHR   presentMode;
    VkExtent2D         extent;

    const std::vector<ImageView::U> imageViews;

    uint32_t GetImageCount () const
    {
        return imageCount;
    }

private:
    struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR        capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR>   presentModes;
    };


    static VkSurfaceFormatKHR ChooseSwapSurfaceFormat (const std::vector<VkSurfaceFormatKHR>& formats)
    {
        if (ERROR (formats.empty ())) {
            throw std::runtime_error ("empty surface format array");
        }

        for (const VkSurfaceFormatKHR& availableFormat : formats) {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                return availableFormat;
            }
        }

        throw std::runtime_error ("failed to choose swapchain surface format");
    }


    static VkPresentModeKHR ChooseSwapPresentMode (const std::vector<VkPresentModeKHR>& modes)
    {
        if (ERROR (modes.empty ())) {
            throw std::runtime_error ("empty present modes array");
        }

        for (VkPresentModeKHR availablePresentMode : modes) {
            if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) { // use VK_PRESENT_MODE_FIFO_KHR for frame limiting ??
                return availablePresentMode;
            }
        }

        throw std::runtime_error ("failed to choose swapchain present mode");
    }


    static VkExtent2D ChooseSwapExtent (const VkSurfaceCapabilitiesKHR& capabilities)
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

    static VkSwapchainKHR CreateSwapchain (VkPhysicalDevice physicalDevice, VkDevice device, VkSurfaceKHR surface, PhysicalDevice::QueueFamilies queueFamilyIndices,
                                           VkSurfaceFormatKHR& surfaceFormat,
                                           VkPresentModeKHR&   presentMode,
                                           VkExtent2D&         extent,
                                           uint32_t&           imageCount)
    {
        SwapChainSupportDetails details;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR (physicalDevice, surface, &details.capabilities);
        uint32_t formatCount = 0;
        vkGetPhysicalDeviceSurfaceFormatsKHR (physicalDevice, surface, &formatCount, nullptr);
        details.formats.resize (formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR (physicalDevice, surface, &formatCount, details.formats.data ());
        uint32_t presentModeCount = 0;
        vkGetPhysicalDeviceSurfacePresentModesKHR (physicalDevice, surface, &presentModeCount, nullptr);
        details.presentModes.resize (presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR (physicalDevice, surface, &presentModeCount, details.presentModes.data ());

        surfaceFormat = ChooseSwapSurfaceFormat (details.formats);
        presentMode   = ChooseSwapPresentMode (details.presentModes);
        extent        = ChooseSwapExtent (details.capabilities);

        imageCount = details.capabilities.minImageCount + 1;
        if (details.capabilities.maxImageCount > 0 && imageCount > details.capabilities.maxImageCount) {
            imageCount = details.capabilities.maxImageCount;
        }

        VkSwapchainCreateInfoKHR createInfo = {};
        createInfo.sType                    = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface                  = surface;
        createInfo.minImageCount            = imageCount;
        createInfo.imageFormat              = surfaceFormat.format;
        createInfo.imageColorSpace          = surfaceFormat.colorSpace;
        createInfo.imageExtent              = extent;
        createInfo.imageArrayLayers         = 1;
        createInfo.imageUsage               = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        uint32_t queueFamilyIndicesData[] = {*queueFamilyIndices.graphics, *queueFamilyIndices.presentation};
        if (ERROR (*queueFamilyIndices.graphics != *queueFamilyIndices.presentation)) {
            createInfo.imageSharingMode      = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices   = queueFamilyIndicesData;
        } else {
            createInfo.imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE;
            createInfo.queueFamilyIndexCount = 0;       // Optional
            createInfo.pQueueFamilyIndices   = nullptr; // Optional
        }
        createInfo.preTransform   = details.capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode    = presentMode;
        createInfo.clipped        = VK_TRUE;
        createInfo.oldSwapchain   = VK_NULL_HANDLE;

        VkSwapchainKHR result;

        if (ERROR (vkCreateSwapchainKHR (device, &createInfo, nullptr, &result) != VK_SUCCESS)) {
            throw std::runtime_error ("failed to create swapchain");
        }

        return result;
    }

    static std::vector<ImageView::U> CreateSwapchainImageViews (VkDevice device, VkSwapchainKHR swapchain, VkFormat format)
    {
        uint32_t imageCount;
        vkGetSwapchainImagesKHR (device, swapchain, &imageCount, nullptr);
        std::vector<VkImage> swapChainImages (imageCount);
        vkGetSwapchainImagesKHR (device, swapchain, &imageCount, swapChainImages.data ());

        std::vector<ImageView::U> result;

        for (size_t i = 0; i < swapChainImages.size (); ++i) {
            result.push_back (ImageView::Create (device, swapChainImages[i], format));
        }

        return result;
    }

public:
    Swapchain (const PhysicalDevice& physicalDevice, VkDevice device, VkSurfaceKHR surface)
        : Swapchain (physicalDevice, device, surface, physicalDevice.queueFamilies)
    {
    }

    Swapchain (VkPhysicalDevice physicalDevice, VkDevice device, VkSurfaceKHR surface, PhysicalDevice::QueueFamilies queueFamilyIndices)
        : physicalDevice (physicalDevice)
        , device (device)
        , handle (CreateSwapchain (physicalDevice, device, surface, queueFamilyIndices, surfaceFormat, presentMode, extent, imageCount))
        , imageViews (CreateSwapchainImageViews (device, handle, surfaceFormat.format))
    {
    }

    ~Swapchain ()
    {
        vkDestroySwapchainKHR (device, handle, nullptr);
        handle = VK_NULL_HANDLE;
    }

    operator VkSwapchainKHR () const
    {
        return handle;
    }
};


#endif