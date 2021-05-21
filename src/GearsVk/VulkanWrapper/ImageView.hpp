#ifndef IMAGEVIEW_HPP
#define IMAGEVIEW_HPP

#include "Assert.hpp"
#include "Image.hpp"
#include "MovablePtr.hpp"
#include "Utils.hpp"
#include "VulkanObject.hpp"

#include <vulkan/vulkan.h>

namespace GVK {

class GVK_RENDERER_API ImageViewBase : public VulkanObject {
private:
    VkDevice                     device;
    VkFormat                     format;
    GVK::MovablePtr<VkImageView> handle;

public:
    ImageViewBase (VkDevice device, VkImage image, VkFormat format, VkImageViewType viewType, uint32_t layerIndex = 0, uint32_t layerCount = 1);
    ImageViewBase (VkDevice device, const Image2D& image, VkImageViewType viewType, uint32_t layerIndex = 0);

    ImageViewBase (ImageViewBase&&) = default;
    ImageViewBase& operator= (ImageViewBase&&) = default;

    virtual ~ImageViewBase () override;

    VkFormat GetFormat () const { return format; }

    operator VkImageView () const { return handle; }
};


class GVK_RENDERER_API ImageView1D : public ImageViewBase {
public:
    ImageView1D (VkDevice device, VkImage image, VkFormat format, uint32_t layerIndex = 0);
    ImageView1D (VkDevice device, const Image& image, uint32_t layerIndex = 0);
};


class GVK_RENDERER_API ImageView2D : public ImageViewBase {
public:
    ImageView2D (VkDevice device, VkImage image, VkFormat format, uint32_t layerIndex = 0, uint32_t layerCount = 1);
    ImageView2D (VkDevice device, const Image& image, uint32_t layerIndex = 0, uint32_t layerCount = 1);
};

class GVK_RENDERER_API ImageView2DArray : public ImageViewBase {
public:
    ImageView2DArray (VkDevice device, VkImage image, VkFormat format, uint32_t layerIndex = 0, uint32_t layerCount = 1);
    ImageView2DArray (VkDevice device, const Image& image, uint32_t layerIndex = 0, uint32_t layerCount = 1);
};


class GVK_RENDERER_API ImageView3D : public ImageViewBase {
public:
    ImageView3D (VkDevice device, VkImage image, VkFormat format, uint32_t layerIndex = 0);
    ImageView3D (VkDevice device, const Image& image, uint32_t layerIndex = 0);
};


class GVK_RENDERER_API ImageViewCube : public ImageViewBase {
public:
    ImageViewCube (VkDevice device, VkImage image, VkFormat format, uint32_t layerIndex = 0);
    ImageViewCube (VkDevice device, const Image& image, uint32_t layerIndex = 0);
};

} // namespace GVK

#endif