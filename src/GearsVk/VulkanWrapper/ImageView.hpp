#ifndef IMAGEVIEW_HPP
#define IMAGEVIEW_HPP

#include "Assert.hpp"
#include "Image.hpp"
#include "Noncopyable.hpp"
#include "Ptr.hpp"
#include "Utils.hpp"
#include "VulkanObject.hpp"

#include <vulkan/vulkan.h>

USING_PTR (ImageViewBase);
class GEARSVK_API ImageViewBase : public VulkanObject {
private:
    const VkDevice device;
    const VkFormat format;
    VkImageView    handle;

public:
    USING_CREATE (ImageViewBase);

    ImageViewBase (VkDevice device, VkImage image, VkFormat format, VkImageViewType viewType, uint32_t layerIndex = 0, uint32_t layerCount = 1)
        : device (device)
        , format (format)
        , handle (VK_NULL_HANDLE)
    {
        VkImageViewCreateInfo createInfo           = {};
        createInfo.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.flags                           = 0;
        createInfo.image                           = image;
        createInfo.viewType                        = viewType;
        createInfo.format                          = format;
        createInfo.components.r                    = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g                    = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b                    = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a                    = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel   = 0;
        createInfo.subresourceRange.levelCount     = 1;
        createInfo.subresourceRange.baseArrayLayer = layerIndex;
        createInfo.subresourceRange.layerCount     = layerCount;

        VkResult result = vkCreateImageView (device, &createInfo, nullptr, &handle);
        if (GVK_ERROR (result != VK_SUCCESS)) {
            throw std::runtime_error ("failed to create swapchain image views");
        }
    }

    ImageViewBase (VkDevice device, const Image2D& image, VkImageViewType viewType, uint32_t layerIndex = 0)
        : ImageViewBase (device, image, image.GetFormat (), viewType, layerIndex)
    {
    }

    virtual ~ImageViewBase ()
    {
        vkDestroyImageView (device, handle, nullptr);
        handle = VK_NULL_HANDLE;
    }

    VkFormat GetFormat () const { return format; }

    operator VkImageView () const { return handle; }
};


USING_PTR (ImageView1D);
class GEARSVK_API ImageView1D : public ImageViewBase {
public:
    USING_CREATE (ImageView1D);

    ImageView1D (VkDevice device, VkImage image, VkFormat format, uint32_t layerIndex = 0)
        : ImageViewBase (device, image, format, VK_IMAGE_VIEW_TYPE_1D, layerIndex)
    {
    }

    ImageView1D (VkDevice device, const ImageBase& image, uint32_t layerIndex = 0)
        : ImageView1D (device, image, image.GetFormat (), layerIndex)
    {
    }
};


USING_PTR (ImageView2D);
class GEARSVK_API ImageView2D : public ImageViewBase {
public:
    USING_CREATE (ImageView2D);

    ImageView2D (VkDevice device, VkImage image, VkFormat format, uint32_t layerIndex = 0, uint32_t layerCount = 1)
        : ImageViewBase (device, image, format, VK_IMAGE_VIEW_TYPE_2D, layerIndex, layerCount)
    {
    }

    ImageView2D (VkDevice device, const ImageBase& image, uint32_t layerIndex = 0, uint32_t layerCount = 1)
        : ImageView2D (device, image, image.GetFormat (), layerIndex, layerCount)
    {
    }
};

USING_PTR (ImageView2DArray);
class GEARSVK_API ImageView2DArray : public ImageViewBase {
public:
    USING_CREATE (ImageView2DArray);

    ImageView2DArray (VkDevice device, VkImage image, VkFormat format, uint32_t layerIndex = 0, uint32_t layerCount = 1)
        : ImageViewBase (device, image, format, VK_IMAGE_VIEW_TYPE_2D, layerIndex, layerCount)
    {
    }

    ImageView2DArray (VkDevice device, const ImageBase& image, uint32_t layerIndex = 0, uint32_t layerCount = 1)
        : ImageView2DArray (device, image, image.GetFormat (), layerIndex, layerCount)
    {
    }
};


USING_PTR (ImageView3D);
class GEARSVK_API ImageView3D : public ImageViewBase {
public:
    USING_CREATE (ImageView3D);

    ImageView3D (VkDevice device, VkImage image, VkFormat format, uint32_t layerIndex = 0)
        : ImageViewBase (device, image, format, VK_IMAGE_VIEW_TYPE_3D, layerIndex)
    {
    }

    ImageView3D (VkDevice device, const ImageBase& image, uint32_t layerIndex = 0)
        : ImageView3D (device, image, image.GetFormat (), layerIndex)
    {
    }
};


USING_PTR (ImageViewCube);
class GEARSVK_API ImageViewCube : public ImageViewBase {
public:
    USING_CREATE (ImageViewCube);

    ImageViewCube (VkDevice device, VkImage image, VkFormat format, uint32_t layerIndex = 0)
        : ImageViewBase (device, image, format, VK_IMAGE_VIEW_TYPE_CUBE, layerIndex)
    {
    }

    ImageViewCube (VkDevice device, const ImageBase& image, uint32_t layerIndex = 0)
        : ImageViewCube (device, image, image.GetFormat (), layerIndex)
    {
    }
};

#endif