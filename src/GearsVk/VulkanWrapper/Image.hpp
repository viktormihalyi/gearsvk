#ifndef IMAGE_HPP
#define IMAGE_HPP

#include "CommandBuffer.hpp"
#include "MovablePtr.hpp"
#include "Utils.hpp"
#include "VulkanObject.hpp"

#include "vk_mem_alloc.h"

#include <vulkan/vulkan.h>

namespace GVK {

class ImageBuilder;

class GVK_RENDERER_API Image : public VulkanObject {
public:
    static const VkImageLayout INITIAL_LAYOUT = VK_IMAGE_LAYOUT_UNDEFINED;

    static constexpr VkFormat R    = VK_FORMAT_R8_UINT;
    static constexpr VkFormat RG   = VK_FORMAT_R8G8_UINT;
    static constexpr VkFormat RGB  = VK_FORMAT_R8G8B8_UINT;
    static constexpr VkFormat RGBA = VK_FORMAT_R8G8B8A8_UINT;

    enum class MemoryLocation {
        GPU,
        CPU
    };

protected:
    GVK::MovablePtr<VkImage> handle;

private:
    VkDevice      device;
    VmaAllocator  allocator;
    VmaAllocation allocationHandle;

    VkFormat format;
    uint32_t width;
    uint32_t height;
    uint32_t depth;
    uint32_t arrayLayers;

protected:
    // for InheritedImage
    Image (VkImage handle, VkDevice device, uint32_t width, uint32_t height, uint32_t depth, VkFormat format, uint32_t arrayLayers);

public:
    Image (VmaAllocator      allocator,
           VkImageType       imageType,
           uint32_t          width,
           uint32_t          height,
           uint32_t          depth,
           VkFormat          format,
           VkImageTiling     tiling,
           VkImageUsageFlags usage,
           uint32_t          arrayLayers,
           MemoryLocation    loc);

    Image (ImageBuilder&);

    Image (Image&&) = default;
    Image& operator= (Image&&) = default;

    virtual ~Image () override;

    VkFormat GetFormat () const { return format; }
    uint32_t GetArrayLayers () const { return arrayLayers; }
    uint32_t GetWidth () const { return width; }
    uint32_t GetHeight () const { return height; }
    uint32_t GetDepth () const { return depth; }

    operator VkImage () const { return handle; }
    operator VmaAllocation () const { return allocationHandle; }

    VkBufferImageCopy GetFullBufferImageCopy () const;

    VkImageMemoryBarrier GetBarrier (VkImageLayout oldLayout, VkImageLayout newLayout) const;

    void CmdPipelineBarrier (CommandBuffer& commandBuffer, VkImageLayout oldLayout, VkImageLayout newLayout) const;

    void CmdCopyBufferToImage (CommandBuffer& commandBuffer, VkBuffer buffer) const;

    void CmdCopyBufferPartToImage (CommandBuffer& commandBuffer, VkBuffer buffer, VkBufferImageCopy region) const;
};


// used for handling swapchain images as Image
class GVK_RENDERER_API InheritedImage : public Image {
public:
    InheritedImage (VkImage handle, uint32_t width, uint32_t height, uint32_t depth, VkFormat format, uint32_t arrayLayers)
        : Image (handle, VK_NULL_HANDLE, width, height, depth, format, arrayLayers)
    {
    }

    virtual ~InheritedImage () override
    {
        handle = nullptr;
    }
};


class GVK_RENDERER_API ImageBuilder {
public:
    const VmaAllocator allocator;

    std::optional<VkImageType>           imageType;
    std::optional<uint32_t>              width;
    std::optional<uint32_t>              height;
    std::optional<uint32_t>              depth;
    std::optional<VkFormat>              format;
    std::optional<VkImageTiling>         tiling;
    VkImageUsageFlags                    usage;
    std::optional<uint32_t>              arrayLayers;
    std::optional<Image::MemoryLocation> loc;

    ImageBuilder (VmaAllocator allocator)
        : allocator (allocator)
        , usage (0)
    {
    }

    // clang-format off

    ImageBuilder& SetMemoryLocation (Image::MemoryLocation value) { loc = value; return *this; }
    ImageBuilder& SetCPU () { loc = Image::MemoryLocation::CPU; return *this; }
    ImageBuilder& SetGPU () { loc = Image::MemoryLocation::GPU; return *this; }

    ImageBuilder& SetImageType (VkImageType value) { imageType = value; return *this; }
    ImageBuilder& Set1D () { imageType = VK_IMAGE_TYPE_1D; return *this; }
    ImageBuilder& Set2D () { imageType = VK_IMAGE_TYPE_2D; return *this; }
    ImageBuilder& Set3D () { imageType = VK_IMAGE_TYPE_3D; return *this; }

    ImageBuilder& SetWidth (uint32_t value) { width = value; return *this; }
    ImageBuilder& SetHeight (uint32_t value) { height = value; return *this; }
    ImageBuilder& SetDepth (uint32_t value) { depth = value; return *this; }

    ImageBuilder& SetFormat (VkFormat value) { format = value; return *this; }

    ImageBuilder& SetTiling (VkImageTiling value) { tiling = value; return *this; }
    ImageBuilder& SetTilingLinear () { tiling = VK_IMAGE_TILING_LINEAR; return *this; }
    ImageBuilder& SetTilingOptimal () { tiling = VK_IMAGE_TILING_OPTIMAL; return *this; }

    ImageBuilder& AddUsageFlag (VkImageUsageFlags flag) { usage |= flag; return *this; }

    ImageBuilder& SetLayers (uint32_t value) { arrayLayers = value; return *this; }

    // clang-format on

    Image Build () const
    {
        const bool allSet = imageType.has_value () &&
                            width.has_value () &&
                            height.has_value () &&
                            depth.has_value () &&
                            format.has_value () &&
                            tiling.has_value () &&
                            usage != 0 &&
                            arrayLayers.has_value () &&
                            loc.has_value ();

        if (GVK_ERROR (!allSet)) {
            throw std::runtime_error ("not all values set");
        }

        return Image (
            allocator,
            *imageType,
            *width,
            *height,
            *depth,
            *format,
            *tiling,
            usage,
            *arrayLayers,
            *loc);
    }
};


class GVK_RENDERER_API Image1D : public Image {
public:
    Image1D (VmaAllocator allocator, MemoryLocation loc, uint32_t width, VkFormat format, VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL, VkImageUsageFlags usage = 0, uint32_t arrayLayers = 1)
        : Image (allocator, VK_IMAGE_TYPE_1D, width, 1, 1, format, tiling, usage, arrayLayers, loc)
    {
    }
};


class GVK_RENDERER_API Image2D : public Image {
public:
    Image2D (VmaAllocator allocator, MemoryLocation loc, uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL, VkImageUsageFlags usage = 0, uint32_t arrayLayers = 1)
        : Image (allocator, VK_IMAGE_TYPE_2D, width, height, 1, format, tiling, usage, arrayLayers, loc)
    {
    }
};


class GVK_RENDERER_API Image3D : public Image {
public:
    Image3D (VmaAllocator allocator, MemoryLocation loc, uint32_t width, uint32_t height, uint32_t depth, VkFormat format, VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL, VkImageUsageFlags usage = 0)
        : Image (allocator, VK_IMAGE_TYPE_3D, width, height, depth, format, tiling, usage, 1, loc)
    {
    }
};

} // namespace GVK

#endif