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
    virtual void                 Recreate ()                                                                                        = 0;
};

class OutOfDateSwapchain : public std::runtime_error {
public:
    OutOfDateSwapchain ()
        : std::runtime_error ("out of date swapchain detected")
    {
    }
};

class RealSwapchain : public Swapchain,
                      public Noncopyable {
public:
    static const VkImageUsageFlags ImageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

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

        std::vector<VkImage>        images;
        std::vector<ImageView::U>   imageViews;
        std::vector<Framebuffer::U> framebuffers;

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

    const CreateSettings createSettings;
    CreateResult         createResult;

public:
    USING_PTR (RealSwapchain);

    RealSwapchain (const PhysicalDevice& physicalDevice, VkDevice device, VkSurfaceKHR surface)
        : RealSwapchain (physicalDevice, device, surface, physicalDevice.GetQueueFamilies ())
    {
    }

    static CreateResult CreateForResult (const CreateSettings& createSettings)
    {
        CreateResult createResult;

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

        createResult.surfaceFormat = createSettings.settings.SelectSurfaceFormat (formats);
        createResult.presentMode   = createSettings.settings.SelectPresentMode (presentModes);
        createResult.extent        = createSettings.settings.SelectExtent (capabilities);
        createResult.imageCount    = createSettings.settings.SelectImageCount (capabilities);

        VkSwapchainCreateInfoKHR createInfo = {};
        createInfo.sType                    = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface                  = createSettings.surface;
        createInfo.minImageCount            = createResult.imageCount;
        createInfo.imageFormat              = createResult.surfaceFormat.format;
        createInfo.imageColorSpace          = createResult.surfaceFormat.colorSpace;
        createInfo.imageExtent              = createResult.extent;
        createInfo.imageArrayLayers         = 1;
        createInfo.imageUsage               = RealSwapchain::ImageUsage;

        uint32_t queueFamilyIndicesData[] = {*createSettings.queueFamilyIndices.graphics, *createSettings.queueFamilyIndices.presentation};
        if (createSettings.queueFamilyIndices.presentation) {
            ASSERT (*createSettings.queueFamilyIndices.graphics == *createSettings.queueFamilyIndices.presentation);
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
        createInfo.presentMode    = createResult.presentMode;
        createInfo.clipped        = VK_TRUE;
        createInfo.oldSwapchain   = VK_NULL_HANDLE;

        if (ERROR (vkCreateSwapchainKHR (createSettings.device, &createInfo, nullptr, &createResult.handle) != VK_SUCCESS)) {
            throw std::runtime_error ("failed to create swapchain");
        }

        uint32_t imageCount;
        vkGetSwapchainImagesKHR (createSettings.device, createResult.handle, &imageCount, nullptr);
        createResult.images.resize (imageCount);
        vkGetSwapchainImagesKHR (createSettings.device, createResult.handle, &imageCount, createResult.images.data ());

        for (size_t i = 0; i < createResult.images.size (); ++i) {
            createResult.imageViews.push_back (ImageView::Create (createSettings.device, createResult.images[i], createResult.surfaceFormat.format));
        }
        return createResult;
    }

    RealSwapchain (VkPhysicalDevice physicalDevice, VkDevice device, VkSurfaceKHR surface, PhysicalDevice::QueueFamilies queueFamilyIndices, SwapchainSettingsProvider& settings = defaultSwapchainSettings)
        : createSettings ({physicalDevice, device, surface, queueFamilyIndices, settings})
    {
        Recreate ();
    }

    virtual void Recreate () override
    {
        if (createResult.handle != VK_NULL_HANDLE) {
            vkDestroySwapchainKHR (createSettings.device, createResult.handle, nullptr);
        }
        createResult.Clear ();
        createResult = CreateForResult (createSettings);
    }

    ~RealSwapchain ()
    {
        vkDestroySwapchainKHR (createSettings.device, createResult.handle, nullptr);
        createResult.Clear ();
    }

    operator VkSwapchainKHR () const { return createResult.handle; }

    virtual VkFormat             GetImageFormat () const override { return createResult.surfaceFormat.format; }
    virtual uint32_t             GetImageCount () const override { return createResult.imageCount; }
    virtual uint32_t             GetWidth () const override { return createResult.extent.width; }
    virtual uint32_t             GetHeight () const override { return createResult.extent.height; }
    virtual std::vector<VkImage> GetImages () const override { return createResult.images; }


    virtual uint32_t GetNextImageIndex (VkSemaphore signalSemaphore) const override
    {
        uint32_t result;

        VkResult err = vkAcquireNextImageKHR (createSettings.device, createResult.handle, UINT64_MAX, signalSemaphore, VK_NULL_HANDLE, &result);
        if (ERROR (err != VK_SUCCESS && err != VK_ERROR_OUT_OF_DATE_KHR && err != VK_SUBOPTIMAL_KHR)) {
            throw std::runtime_error ("bro");
        }

        if (err == VK_ERROR_OUT_OF_DATE_KHR) {
            std::cout << "out of date swapchain detected" << std::endl;
            throw OutOfDateSwapchain ();
        }

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
        presentInfo.pSwapchains        = &createResult.handle;
        presentInfo.pImageIndices      = &imageIndex;
        presentInfo.pResults           = nullptr;

        VkResult err = vkQueuePresentKHR (queue, &presentInfo);
        if (ERROR (err != VK_SUCCESS && err != VK_SUBOPTIMAL_KHR)) {
            throw std::runtime_error ("failed to present");
        }

        if (err == VK_SUBOPTIMAL_KHR) {
            throw OutOfDateSwapchain ();
        }
    }
};


class FakeSwapchain : public Swapchain {
public:
    AllocatedImage image;
    Device&        device;
    const uint32_t width;
    const uint32_t height;

    USING_PTR (FakeSwapchain);

    FakeSwapchain (Device& device, VkQueue queue, VkCommandPool commandPool, uint32_t width, uint32_t height)
        : device (device)
        , width (width)
        , height (height)
        , image (device, Image::Create (device, width, height, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, RealSwapchain::ImageUsage, 1), DeviceMemory::GPU)
    {
        TransitionImageLayout (device, queue, commandPool, *image.image, Image::INITIAL_LAYOUT, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
    }

    virtual VkFormat             GetImageFormat () const override { return image.image->GetFormat (); }
    virtual uint32_t             GetImageCount () const override { return 1; }
    virtual uint32_t             GetWidth () const override { return width; }
    virtual uint32_t             GetHeight () const override { return height; }
    virtual std::vector<VkImage> GetImages () const override { return {*image.image}; }
    virtual void                 Recreate () override {}

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