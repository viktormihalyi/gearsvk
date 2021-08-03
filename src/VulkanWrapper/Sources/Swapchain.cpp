#include "Swapchain.hpp"
#include "VulkanUtils.hpp"

#include <iostream>

#include "spdlog/spdlog.h"

namespace GVK {


const VkImageUsageFlags RealSwapchain::ImageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;


VkSurfaceFormatKHR DefaultSwapchainSettings::SelectSurfaceFormat (const std::vector<VkSurfaceFormatKHR>& formats)
{
    const VkSurfaceFormatKHR preferred { VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };

    for (const VkSurfaceFormatKHR& availableFormat : formats) {
        if (availableFormat.format == preferred.format && availableFormat.colorSpace == preferred.colorSpace) {
            return availableFormat;
        }
    }

    spdlog::error ("VkSwapchainKHR: Failed to choose swapchain surface format ({} format and {} color space is not available).", preferred.format, preferred.colorSpace);
    throw std::runtime_error ("failed to choose swapchain surface format");
}


VkPresentModeKHR DefaultSwapchainSettings::SelectPresentMode (const std::vector<VkPresentModeKHR>& modes)
{
    for (VkPresentModeKHR availablePresentMode : modes) {
        if (availablePresentMode == VK_PRESENT_MODE_FIFO_KHR) {
            return availablePresentMode;
        }
    }

    spdlog::error ("VkSwapchainKHR: Failed to choose swapchain present mode.");
    throw std::runtime_error ("failed to choose swapchain present mode");
}


VkExtent2D DefaultSwapchainSettings::SelectExtent (const VkSurfaceCapabilitiesKHR& capabilities)
{
    if (capabilities.currentExtent.width != UINT32_MAX) {
        return capabilities.currentExtent;
    } else {
        spdlog::info ("VkSwapchainKHR: Trying to choose 800x600 resolution.");
        // TODO
        VkExtent2D actualExtent = { 800, 600 };
        actualExtent.width      = std::clamp (capabilities.minImageExtent.width, capabilities.maxImageExtent.width, actualExtent.width);
        actualExtent.height     = std::clamp (capabilities.minImageExtent.height, capabilities.maxImageExtent.height, actualExtent.height);
        return actualExtent;
    }
}


uint32_t DefaultSwapchainSettings::SelectImageCount (const VkSurfaceCapabilitiesKHR& capabilities)
{
    if (capabilities.minImageCount < 3 && (3 < capabilities.maxImageCount || capabilities.maxImageCount == 0)) {
        return 3;
    }

    const uint32_t minPlusOne = capabilities.minImageCount;
    if (capabilities.minImageCount < minPlusOne && (minPlusOne < capabilities.maxImageCount || capabilities.maxImageCount == 0)) {
        return minPlusOne;
    }

    return capabilities.minImageCount;
}


uint32_t DefaultSwapchainSettingsSingleImage::SelectImageCount (const VkSurfaceCapabilitiesKHR& capabilities)
{
    if (GVK_ERROR (capabilities.minImageCount > 1)) {
        return capabilities.minImageCount;
    }

    return 1;
}


RealSwapchain::CreateResult RealSwapchain::CreateForResult (const CreateSettings& createSettings)
{
    CreateResult createResult;

    if (createSettings.queueFamilyIndices.presentation.has_value ()) {
        VkBool32 yes;
        VkResult yesyes = vkGetPhysicalDeviceSurfaceSupportKHR (createSettings.physicalDevice, *createSettings.queueFamilyIndices.presentation, createSettings.surface, &yes);
        if (GVK_ERROR (yesyes != VK_SUCCESS || yes != VK_TRUE)) {
            spdlog::error ("VkSwapchainKHR: Cannot create swapchain for this surface on this physical device.");
            throw std::runtime_error ("cannot create swapchain on this physicald device queue");
        }
    }

    VkSurfaceCapabilitiesKHR capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR (createSettings.physicalDevice, createSettings.surface, &capabilities);

    std::vector<VkSurfaceFormatKHR> formats;
    uint32_t                        formatCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR (createSettings.physicalDevice, createSettings.surface, &formatCount, nullptr);
    formats.resize (formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR (createSettings.physicalDevice, createSettings.surface, &formatCount, formats.data ());

    std::vector<VkPresentModeKHR> presentModes;
    uint32_t                      presentModeCount = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR (createSettings.physicalDevice, createSettings.surface, &presentModeCount, nullptr);
    presentModes.resize (presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR (createSettings.physicalDevice, createSettings.surface, &presentModeCount, presentModes.data ());

    createResult.surfaceFormat = createSettings.settings->SelectSurfaceFormat (formats);
    createResult.presentMode   = createSettings.settings->SelectPresentMode (presentModes);
    createResult.extent        = createSettings.settings->SelectExtent (capabilities);
    createResult.imageCount    = createSettings.settings->SelectImageCount (capabilities);

    VkSwapchainCreateInfoKHR createInfo = {};
    createInfo.sType                    = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface                  = createSettings.surface;
    createInfo.minImageCount            = createResult.imageCount;
    createInfo.imageFormat              = createResult.surfaceFormat.format;
    createInfo.imageColorSpace          = createResult.surfaceFormat.colorSpace;
    createInfo.imageExtent              = createResult.extent;
    createInfo.imageArrayLayers         = 1;
    createInfo.imageUsage               = RealSwapchain::ImageUsage;

    uint32_t queueFamilyIndicesData[] = { *createSettings.queueFamilyIndices.graphics, *createSettings.queueFamilyIndices.presentation };
    if (createSettings.queueFamilyIndices.presentation) {
        GVK_ASSERT (*createSettings.queueFamilyIndices.graphics == *createSettings.queueFamilyIndices.presentation);
    }

    //if (GVK_ERROR (*queueFamilyIndices.graphics != *queueFamilyIndices.presentation)) {
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
    createInfo.presentMode    = createResult.presentMode;
    createInfo.clipped        = VK_TRUE;
    createInfo.oldSwapchain   = VK_NULL_HANDLE;

    if (GVK_ERROR (vkCreateSwapchainKHR (createSettings.device, &createInfo, nullptr, &createResult.handle) != VK_SUCCESS)) {
        spdlog::critical ("VkSwapchainKHR creation failed.");
        throw std::runtime_error ("failed to create swapchain");
    }

    uint32_t imageCount;
    vkGetSwapchainImagesKHR (createSettings.device, createResult.handle, &imageCount, nullptr);
    createResult.images.resize (imageCount);
    vkGetSwapchainImagesKHR (createSettings.device, createResult.handle, &imageCount, createResult.images.data ());

    for (size_t i = 0; i < createResult.images.size (); ++i) {
        createResult.imageViews.push_back (std::make_unique<ImageView2D> (createSettings.device, createResult.images[i], createResult.surfaceFormat.format));
    }
    return createResult;
}


RealSwapchain::RealSwapchain (const PhysicalDevice& physicalDevice, VkDevice device, VkSurfaceKHR surface, std::unique_ptr<SwapchainSettingsProvider>&& settings)
    : RealSwapchain (physicalDevice, device, surface, physicalDevice.GetQueueFamilies (), std::move (settings))
{
}


RealSwapchain::RealSwapchain (VkPhysicalDevice physicalDevice, VkDevice device, VkSurfaceKHR surface, PhysicalDevice::QueueFamilies queueFamilyIndices, std::unique_ptr<SwapchainSettingsProvider>&& settings)
    : createSettings ({ physicalDevice, device, surface, queueFamilyIndices, std::move (settings) })
{
    Recreate ();
}


void RealSwapchain::Recreate ()
{
    if (createResult.handle != VK_NULL_HANDLE) {
        vkDestroySwapchainKHR (createSettings.device, createResult.handle, nullptr);
    }
    createResult = CreateForResult (createSettings);

    spdlog::trace ("VkSwapchainKHR created: {}, uuid: {}.", createResult.handle, GetUUID ().GetValue ());
}


void RealSwapchain::RecreateForSurface (VkSurfaceKHR surface)
{
    createSettings.surface = surface;
    Recreate ();
}


bool RealSwapchain::IsEqualSettings (const Swapchain& other)
{
    GVK_ASSERT (dynamic_cast<const RealSwapchain*> (&other) != nullptr);
    return createResult.IsEqualSettings (static_cast<const RealSwapchain&> (other).createResult);
}


RealSwapchain::~RealSwapchain ()
{
    if (createResult.handle != VK_NULL_HANDLE) {
        vkDestroySwapchainKHR (createSettings.device, createResult.handle, nullptr);
        createResult.Clear ();
    }
}


std::vector<std::unique_ptr<InheritedImage>> RealSwapchain::GetImageObjects () const
{
    std::vector<std::unique_ptr<InheritedImage>> result;

    for (uint32_t swapchainImageIndex = 0; swapchainImageIndex < GetImageCount (); ++swapchainImageIndex) {
        result.push_back (std::make_unique<InheritedImage> (
            createResult.images[swapchainImageIndex],
            GetWidth (),
            GetHeight (),
            1,
            GetImageFormat (),
            1));
    }

    return result;
}


uint32_t RealSwapchain::GetNextImageIndex (VkSemaphore signalSemaphore, VkFence fenceToSignal) const
{
    uint32_t result;

    VkResult err = vkAcquireNextImageKHR (createSettings.device, createResult.handle, UINT64_MAX, signalSemaphore, fenceToSignal, &result);
    if (GVK_ERROR (err != VK_SUCCESS && err != VK_ERROR_OUT_OF_DATE_KHR && err != VK_SUBOPTIMAL_KHR)) {
        spdlog::error ("VkSwapchain: vkAcquireNextImageKHR failed.");
        throw std::runtime_error ("err");
    }

    if (err == VK_ERROR_OUT_OF_DATE_KHR) {
        spdlog::debug ("VkSwapchain: Out of date swaphchain detected.");
        throw OutOfDateSwapchain { *this };
    }

    return result;
}


void RealSwapchain::Present (VkQueue queue, uint32_t imageIndex, const std::vector<VkSemaphore>& waitSemaphores) const
{
    VkPresentInfoKHR presentInfo   = {};
    presentInfo.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = static_cast<uint32_t> (waitSemaphores.size ());
    presentInfo.pWaitSemaphores    = waitSemaphores.data ();
    presentInfo.swapchainCount     = 1;
    presentInfo.pSwapchains        = &createResult.handle;
    presentInfo.pImageIndices      = &imageIndex;
    presentInfo.pResults           = nullptr;

    const VkResult err = vkQueuePresentKHR (queue, &presentInfo);
    if (GVK_ERROR (err != VK_SUCCESS && err != VK_SUBOPTIMAL_KHR && err != VK_ERROR_OUT_OF_DATE_KHR)) {
        spdlog::error ("VkSwapchain: Presentation failed.");
        throw std::runtime_error ("failed to present");
    }

    if (err == VK_SUBOPTIMAL_KHR || err == VK_ERROR_OUT_OF_DATE_KHR) {
        spdlog::debug ("VkSwapchain: Out of date swaphchain detected.");
        throw OutOfDateSwapchain { *this };
    }

    spdlog::trace ("VkSwapchain: Presenting on swapchain {}.", createResult.handle);
}


FakeSwapchain::FakeSwapchain (const DeviceExtra& device, uint32_t width, uint32_t height)
    : device (device)
    , width (width)
    , height (height)
    , image (std::make_unique<Image2D> (device.GetAllocator (), Image::MemoryLocation::GPU, width, height, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, RealSwapchain::ImageUsage, 1))
{
    imageViews.push_back (std::make_unique<ImageView2D> (device, *image));
    TransitionImageLayout (device, *image, Image2D::INITIAL_LAYOUT, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
}

} // namespace GVK
