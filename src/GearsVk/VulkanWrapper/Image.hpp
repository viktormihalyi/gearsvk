#ifndef IMAGE_HPP
#define IMAGE_HPP

#include "CommandBuffer.hpp"
#include "Noncopyable.hpp"
#include "Ptr.hpp"
#include "Utils.hpp"
#include "VulkanObject.hpp"

#include "vk_mem_alloc.h"

#include <vulkan/vulkan.h>

class ImageBuilder;

USING_PTR (ImageBase);
class GEARSVK_API ImageBase : public VulkanObject {
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
    VkImage handle;

private:
    const VkDevice     device;
    const VmaAllocator allocator;
    VmaAllocation      allocationHandle;

    const VkFormat format;
    uint32_t       width;
    uint32_t       height;
    uint32_t       depth;
    uint32_t       arrayLayers;

protected:
    // for InheritedImage
    ImageBase (VkImage handle, VkDevice device, uint32_t width, uint32_t height, uint32_t depth, VkFormat format, uint32_t arrayLayers);

public:
    USING_CREATE (ImageBase);

    ImageBase (VmaAllocator      allocator,
               VkImageType       imageType,
               uint32_t          width,
               uint32_t          height,
               uint32_t          depth,
               VkFormat          format,
               VkImageTiling     tiling,
               VkImageUsageFlags usage,
               uint32_t          arrayLayers,
               MemoryLocation    loc);

    ImageBase (ImageBuilder&);

    virtual ~ImageBase () override;

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


// used for handling swapchain images as ImageBase
USING_PTR (InheritedImage);
class GEARSVK_API InheritedImage : public ImageBase {
    USING_CREATE (InheritedImage);

public:
    InheritedImage (VkImage handle, uint32_t width, uint32_t height, uint32_t depth, VkFormat format, uint32_t arrayLayers)
        : ImageBase (handle, VK_NULL_HANDLE, width, height, depth, format, arrayLayers)
    {
    }

    virtual ~InheritedImage () override
    {
        handle = VK_NULL_HANDLE;
    }
};


class ImageBuilder {
public:
    const VmaAllocator allocator;

    std::optional<VkImageType>               imageType;
    std::optional<uint32_t>                  width;
    std::optional<uint32_t>                  height;
    std::optional<uint32_t>                  depth;
    std::optional<VkFormat>                  format;
    std::optional<VkImageTiling>             tiling;
    VkImageUsageFlags                        usage;
    std::optional<uint32_t>                  arrayLayers;
    std::optional<ImageBase::MemoryLocation> loc;

    ImageBuilder (VmaAllocator allocator)
        : allocator (allocator)
        , usage (0)
    {
    }

    // clang-format off

    ImageBuilder& SetMemoryLocation (ImageBase::MemoryLocation value) { loc = value; return *this; }
    ImageBuilder& SetCPU () { loc = ImageBase::MemoryLocation::CPU; return *this; }
    ImageBuilder& SetGPU () { loc = ImageBase::MemoryLocation::GPU; return *this; }

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

    ImageBase&& Build () const
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

        return ImageBase (
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


USING_PTR (Image1D);
class GEARSVK_API Image1D : public ImageBase {
    USING_CREATE (Image1D);

public:
    Image1D (VmaAllocator allocator, MemoryLocation loc, uint32_t width, VkFormat format, VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL, VkImageUsageFlags usage = 0, uint32_t arrayLayers = 1)
        : ImageBase (allocator, VK_IMAGE_TYPE_1D, width, 1, 1, format, tiling, usage, arrayLayers, loc)
    {
    }
};


USING_PTR (Image2D);
class GEARSVK_API Image2D : public ImageBase {
    USING_CREATE (Image2D);

public:
    Image2D (VmaAllocator allocator, MemoryLocation loc, uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL, VkImageUsageFlags usage = 0, uint32_t arrayLayers = 1)
        : ImageBase (allocator, VK_IMAGE_TYPE_2D, width, height, 1, format, tiling, usage, arrayLayers, loc)
    {
    }
};


USING_PTR (Image3D);
class GEARSVK_API Image3D : public ImageBase {
    USING_CREATE (Image3D);

public:
    Image3D (VmaAllocator allocator, MemoryLocation loc, uint32_t width, uint32_t height, uint32_t depth, VkFormat format, VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL, VkImageUsageFlags usage = 0)
        : ImageBase (allocator, VK_IMAGE_TYPE_3D, width, height, depth, format, tiling, usage, 1, loc)
    {
    }
};

#endif