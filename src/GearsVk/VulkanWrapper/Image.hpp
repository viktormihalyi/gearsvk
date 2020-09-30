#ifndef IMAGE_HPP
#define IMAGE_HPP

#include "Assert.hpp"
#include "CommandBuffer.hpp"
#include "Noncopyable.hpp"
#include "Ptr.hpp"
#include "Utils.hpp"
#include "VulkanObject.hpp"

#include "vk_mem_alloc.h"

#include <vulkan/vulkan.h>

USING_PTR (ImageBase);
class GEARSVK_API ImageBase : public VulkanObject {
public:
    static const VkImageLayout INITIAL_LAYOUT = VK_IMAGE_LAYOUT_UNDEFINED;

    static constexpr VkFormat R    = VK_FORMAT_R8_UINT;
    static constexpr VkFormat RG   = VK_FORMAT_R8G8_UINT;
    static constexpr VkFormat RGB  = VK_FORMAT_R8G8B8_UINT;
    static constexpr VkFormat RGBA = VK_FORMAT_R8G8B8A8_UINT;

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
    ImageBase (VkImage handle, VkDevice device, uint32_t width, uint32_t height, uint32_t depth, VkFormat format, uint32_t arrayLayers)
        : handle (handle)
        , allocator (VK_NULL_HANDLE)
        , allocationHandle (VK_NULL_HANDLE)
        , device (device)
        , format (format)
        , width (width)
        , height (height)
        , depth (depth)
        , arrayLayers (arrayLayers)
    {
    }

public:
    USING_CREATE (ImageBase);


    ImageBase (VkDevice device, VkImageType imageType, uint32_t width, uint32_t height, uint32_t depth, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, uint32_t arrayLayers)
        : device (device)
        , allocator (VK_NULL_HANDLE)
        , allocationHandle (VK_NULL_HANDLE)
        , handle (VK_NULL_HANDLE)
        , format (format)
        , width (width)
        , height (height)
        , depth (depth)
        , arrayLayers (arrayLayers)
    {
        VkImageCreateInfo imageInfo = {};
        imageInfo.sType             = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.flags             = 0;
        imageInfo.imageType         = imageType;
        imageInfo.extent.width      = width;
        imageInfo.extent.height     = height;
        imageInfo.extent.depth      = depth;
        imageInfo.mipLevels         = 1;
        imageInfo.arrayLayers       = arrayLayers;
        imageInfo.format            = format;
        imageInfo.tiling            = tiling;
        imageInfo.initialLayout     = INITIAL_LAYOUT;
        imageInfo.usage             = usage;
        imageInfo.samples           = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.sharingMode       = VK_SHARING_MODE_EXCLUSIVE;

        if (GVK_ERROR (vkCreateImage (device, &imageInfo, nullptr, &handle) != VK_SUCCESS)) {
            throw std::runtime_error ("failed to create image!");
        }
    }

    enum class MemoryLocation {
        GPU,
        CPU
    };

    ImageBase (VmaAllocator      allocator,
               VkImageType       imageType,
               uint32_t          width,
               uint32_t          height,
               uint32_t          depth,
               VkFormat          format,
               VkImageTiling     tiling,
               VkImageUsageFlags usage,
               uint32_t          arrayLayers,
               MemoryLocation    loc)
        : device (VK_NULL_HANDLE)
        , handle (VK_NULL_HANDLE)
        , allocator (allocator)
        , allocationHandle (VK_NULL_HANDLE)
        , format (format)
        , width (width)
        , height (height)
        , depth (depth)
        , arrayLayers (arrayLayers)
    {
        VkImageCreateInfo imageInfo = {};
        imageInfo.sType             = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.flags             = 0;
        imageInfo.imageType         = imageType;
        imageInfo.extent.width      = width;
        imageInfo.extent.height     = height;
        imageInfo.extent.depth      = depth;
        imageInfo.mipLevels         = 1;
        imageInfo.arrayLayers       = arrayLayers;
        imageInfo.format            = format;
        imageInfo.tiling            = tiling;
        imageInfo.initialLayout     = INITIAL_LAYOUT;
        imageInfo.usage             = usage;
        imageInfo.samples           = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.sharingMode       = VK_SHARING_MODE_EXCLUSIVE;

        VmaAllocationCreateInfo allocInfo = {};
        allocInfo.usage                   = (loc == MemoryLocation::GPU) ? VMA_MEMORY_USAGE_GPU_ONLY : VMA_MEMORY_USAGE_CPU_COPY;

        if (GVK_ERROR (vmaCreateImage (allocator, &imageInfo, &allocInfo, &handle, &allocationHandle, nullptr) != VK_SUCCESS)) {
            throw std::runtime_error ("failed to create image!");
        }
    }

    virtual ~ImageBase ()
    {
        if (handle == VK_NULL_HANDLE) {
            return;
        }

        if (device != VK_NULL_HANDLE) {
            vkDestroyImage (device, handle, nullptr);
        } else if (allocator != VK_NULL_HANDLE) {
            vmaDestroyImage (allocator, handle, allocationHandle);
        }
        handle = VK_NULL_HANDLE;
    }

    VkFormat GetFormat () const { return format; }
    uint32_t GetArrayLayers () const { return arrayLayers; }
    uint32_t GetWidth () const { return width; }
    uint32_t GetHeight () const { return height; }
    uint32_t GetDepth () const { return depth; }

    void CmdPipelineBarrier (CommandBuffer& commandBuffer, VkImageLayout oldLayout, VkImageLayout newLayout) const
    {
        VkImageMemoryBarrier barrier            = {};
        barrier.sType                           = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout                       = oldLayout;
        barrier.newLayout                       = newLayout;
        barrier.srcQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
        barrier.image                           = handle;
        barrier.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel   = 0;
        barrier.subresourceRange.levelCount     = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount     = arrayLayers;
        barrier.srcAccessMask                   = 0;
        barrier.dstAccessMask                   = 0;

        VkPipelineStageFlags sourceStage;
        VkPipelineStageFlags destinationStage;

        if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

            sourceStage      = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            sourceStage      = VK_PIPELINE_STAGE_TRANSFER_BIT;
            destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        } else {
            // TODO
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = 0;
            sourceStage           = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
            destinationStage      = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
        }

        commandBuffer.CmdPipelineBarrier (
            sourceStage,
            destinationStage,
            {},
            {},
            { barrier });
    }

    operator VkImage () const { return handle; }
    operator VmaAllocation () const { return allocationHandle; }

    VkBufferImageCopy GetFullBufferImageCopy () const
    {
        VkBufferImageCopy result               = {};
        result.bufferOffset                    = 0;
        result.bufferRowLength                 = 0;
        result.bufferImageHeight               = 0;
        result.imageSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        result.imageSubresource.mipLevel       = 0;
        result.imageSubresource.baseArrayLayer = 0;
        result.imageSubresource.layerCount     = arrayLayers;
        result.imageOffset                     = { 0, 0, 0 };
        result.imageExtent                     = { width, height, depth };
        return result;
    }

    void CmdCopyBufferToImage (CommandBuffer& commandBuffer, VkBuffer buffer) const
    {
        VkBufferImageCopy region = GetFullBufferImageCopy ();
        commandBuffer.CmdCopyBufferToImage (
            buffer,
            handle,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1,
            &region);
    }

    void CmdCopyBufferPartToImage (CommandBuffer& commandBuffer, VkBuffer buffer, VkBufferImageCopy region) const
    {
        commandBuffer.CmdCopyBufferToImage (
            buffer,
            handle,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1,
            &region);
    }
};


USING_PTR (InheritedImage);
class GEARSVK_API InheritedImage : public ImageBase {
public:
    USING_CREATE (InheritedImage);
    InheritedImage (VkImage handle, uint32_t width, uint32_t height, uint32_t depth, VkFormat format, uint32_t arrayLayers)
        : ImageBase (handle, VK_NULL_HANDLE, width, height, depth, format, arrayLayers)
    {
    }

    virtual ~InheritedImage () override
    {
        handle = VK_NULL_HANDLE;
    }
};


USING_PTR (Image1D);
class GEARSVK_API Image1D : public ImageBase {
public:
    USING_CREATE (Image1D);
    Image1D (VkDevice device, uint32_t width, VkFormat format, VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL, VkImageUsageFlags usage = 0, uint32_t arrayLayers = 1)
        : ImageBase (device, VK_IMAGE_TYPE_1D, width, 1, 1, format, tiling, usage, arrayLayers)
    {
    }
    Image1D (VmaAllocator allocator, MemoryLocation loc, uint32_t width, VkFormat format, VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL, VkImageUsageFlags usage = 0, uint32_t arrayLayers = 1)
        : ImageBase (allocator, VK_IMAGE_TYPE_1D, width, 1, 1, format, tiling, usage, arrayLayers, loc)
    {
    }
};


USING_PTR (Image2D);
class GEARSVK_API Image2D : public ImageBase {
public:
    USING_CREATE (Image2D);
    Image2D (VkDevice device, uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL, VkImageUsageFlags usage = 0, uint32_t arrayLayers = 1)
        : ImageBase (device, VK_IMAGE_TYPE_2D, width, height, 1, format, tiling, usage, arrayLayers)
    {
    }
    Image2D (VmaAllocator allocator, MemoryLocation loc, uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL, VkImageUsageFlags usage = 0, uint32_t arrayLayers = 1)
        : ImageBase (allocator, VK_IMAGE_TYPE_2D, width, height, 1, format, tiling, usage, arrayLayers, loc)
    {
    }
};


USING_PTR (Image3D);
class GEARSVK_API Image3D : public ImageBase {
public:
    USING_CREATE (Image3D);
    Image3D (VkDevice device, uint32_t width, uint32_t height, uint32_t depth, VkFormat format, VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL, VkImageUsageFlags usage = 0)
        : ImageBase (device, VK_IMAGE_TYPE_3D, width, height, depth, format, tiling, usage, 1)
    {
    }
    Image3D (VmaAllocator allocator, MemoryLocation loc, uint32_t width, uint32_t height, uint32_t depth, VkFormat format, VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL, VkImageUsageFlags usage = 0)
        : ImageBase (allocator, VK_IMAGE_TYPE_3D, width, height, depth, format, tiling, usage, 1, loc)
    {
    }
};

#endif