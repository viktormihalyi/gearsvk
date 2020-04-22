#ifndef IMAGEVIEW_HPP
#define IMAGEVIEW_HPP

#include "Assert.hpp"
#include "Image.hpp"
#include "Noncopyable.hpp"
#include "Ptr.hpp"
#include "Utils.hpp"

#include <vulkan/vulkan.h>

class ImageView : public Noncopyable {
private:
    const VkDevice device;
    const VkFormat format;
    VkImageView    handle;

public:
    USING_PTR (ImageView);

    ImageView (VkDevice device, VkImage image, VkFormat format, uint32_t layerIndex = 0)
        : device (device)
        , format (format)
        , handle (VK_NULL_HANDLE)
    {
        VkImageViewCreateInfo createInfo           = {};
        createInfo.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.flags                           = 0;
        createInfo.image                           = image;
        createInfo.viewType                        = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format                          = format;
        createInfo.components.r                    = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g                    = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b                    = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a                    = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel   = 0;
        createInfo.subresourceRange.levelCount     = 1;
        createInfo.subresourceRange.baseArrayLayer = layerIndex;
        createInfo.subresourceRange.layerCount     = 1;

        VkResult result = vkCreateImageView (device, &createInfo, nullptr, &handle);
        if (ERROR (result != VK_SUCCESS)) {
            throw std::runtime_error ("failed to create swapchain image views");
        }
    }

    ImageView (VkDevice device, const Image2D& image, uint32_t layerIndex = 0)
        : ImageView (device, image, image.GetFormat (), layerIndex)
    {
    }

    ~ImageView ()
    {
        vkDestroyImageView (device, handle, nullptr);
        handle = VK_NULL_HANDLE;
    }

    VkFormat GetFormat () const { return format; }

    operator VkImageView () const { return handle; }
};

#endif