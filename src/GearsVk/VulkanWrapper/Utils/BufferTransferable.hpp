#ifndef BUFFER_TRANSFERABLE_HPP
#define BUFFER_TRANSFERABLE_HPP

#include "GearsVkAPI.hpp"

#include "Buffer.hpp"
#include "CommandBuffer.hpp"
#include "Device.hpp"
#include "DeviceMemory.hpp"
#include "Image.hpp"
#include "MemoryMapping.hpp"
#include "Ptr.hpp"
#include "SingleTimeCommand.hpp"
#include "VulkanUtils.hpp"

#include <cstring>


USING_PTR (BufferTransferable);
class GVK_RENDERER_API BufferTransferable final {
public:
    const DeviceExtra& device;

    uint32_t bufferSize;

    Buffer        bufferGPU;
    Buffer        bufferCPU;
    MemoryMapping bufferCPUMapping;

    USING_CREATE (BufferTransferable);

    BufferTransferable (const DeviceExtra& device, uint32_t bufferSize, VkBufferUsageFlags usageFlags)
        : device (device)
        , bufferSize (bufferSize)
        , bufferGPU (device.GetAllocator (), bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | usageFlags, Buffer::MemoryLocation::GPU)
        , bufferCPU (device.GetAllocator (), bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, Buffer::MemoryLocation::CPU)
        , bufferCPUMapping (device.GetAllocator (), bufferCPU)
    {
    }

    void CopyAndTransfer (const void* data, size_t size) const
    {
        GVK_ASSERT (size == bufferSize);
        bufferCPUMapping.Copy (data, size);
        CopyBuffer (device, bufferCPU, bufferGPU, bufferSize);
    }

    VkBuffer GetBufferToBind () const
    {
        return bufferGPU;
    }
};


USING_PTR (ImageTransferable);
class GVK_RENDERER_API ImageTransferable {
    USING_CREATE (ImageTransferable);

private:
    const DeviceExtra& device;

    Buffer        bufferCPU;
    MemoryMapping bufferCPUMapping;

public:
    ImageU imageGPU;

protected:
    ImageTransferable (const DeviceExtra& device, uint32_t bufferSize)
        : device (device)
        , bufferCPU (device.GetAllocator (), bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, Buffer::MemoryLocation::CPU)
        , bufferCPUMapping (device.GetAllocator (), bufferCPU)
    {
    }

public:
    virtual ~ImageTransferable () = default;

    void CopyTransitionTransfer (VkImageLayout currentImageLayout, const void* data, size_t size, std::optional<VkImageLayout> nextLayout = std::nullopt) const
    {
        CopyLayer (currentImageLayout, data, size, 0, nextLayout);
    }

    void CopyLayer (VkImageLayout currentImageLayout, const void* data, size_t size, uint32_t layerIndex, std::optional<VkImageLayout> nextLayout = std::nullopt) const
    {
        bufferCPUMapping.Copy (data, size);

        SingleTimeCommand commandBuffer (device);

        imageGPU->CmdPipelineBarrier (commandBuffer, currentImageLayout, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

        VkBufferImageCopy region               = {};
        region.bufferOffset                    = 0;
        region.bufferRowLength                 = 0;
        region.bufferImageHeight               = 0;
        region.imageSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel       = 0;
        region.imageSubresource.baseArrayLayer = layerIndex;
        region.imageSubresource.layerCount     = 1;
        region.imageOffset                     = { 0, 0, 0 };
        region.imageExtent                     = { imageGPU->GetWidth (), imageGPU->GetHeight (), imageGPU->GetDepth () };

        imageGPU->CmdCopyBufferPartToImage (commandBuffer, bufferCPU, region);
        if (nextLayout.has_value ()) {
            imageGPU->CmdPipelineBarrier (commandBuffer, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, *nextLayout);
        }
    }

    VkImage GetImageToBind () const
    {
        return *imageGPU;
    }
};


static uint32_t GetCompontentCountFromFormat (VkFormat format)
{
    switch (format) {
        case VK_FORMAT_R8_UINT: return 1;
        case VK_FORMAT_R8G8_UINT: return 2;
        case VK_FORMAT_R8G8B8_UINT: return 3;
        case VK_FORMAT_R8G8B8A8_UINT: return 4;
        case VK_FORMAT_R32_UINT: return 4;
        case VK_FORMAT_R32G32_UINT: return 8;
        case VK_FORMAT_R32G32B32_UINT: return 12;
        case VK_FORMAT_R32G32B32A32_UINT: return 16;
        case VK_FORMAT_R8_SRGB: return 1;
        case VK_FORMAT_R8G8_SRGB: return 2;
        case VK_FORMAT_R8G8B8_SRGB: return 3;
        case VK_FORMAT_R8G8B8A8_SRGB: return 4;
        case VK_FORMAT_R32_SFLOAT: return 4;
        case VK_FORMAT_R32G32_SFLOAT: return 8;
        case VK_FORMAT_R32G32B32_SFLOAT: return 12;
        case VK_FORMAT_R32G32B32A32_SFLOAT: return 16;
        default:
            GVK_ASSERT (false);
            return 4;
    }
}


USING_PTR (Image1DTransferable);
class GVK_RENDERER_API Image1DTransferable final : public ImageTransferable {
public:
    USING_CREATE (Image1DTransferable);
    Image1DTransferable (const DeviceExtra& device, VkFormat format, uint32_t width, VkImageUsageFlags usageFlags)
        : ImageTransferable (device, width * GetCompontentCountFromFormat (format))
    {
        imageGPU = Image1D::Create (device.GetAllocator (), Image::MemoryLocation::GPU, width, format, VK_IMAGE_TILING_OPTIMAL, VK_BUFFER_USAGE_TRANSFER_DST_BIT | usageFlags);
    }
};


USING_PTR (Image2DTransferable);
class GVK_RENDERER_API Image2DTransferable final : public ImageTransferable {
public:
    USING_CREATE (Image2DTransferable);
    Image2DTransferable (const DeviceExtra& device, VkFormat format, uint32_t width, uint32_t height, VkImageUsageFlags usageFlags, uint32_t arrayLayers = 1)
        : ImageTransferable (device, width * height * GetCompontentCountFromFormat (format))
    {
        imageGPU = Image2D::Create (device.GetAllocator (), Image::MemoryLocation::GPU, width, height, format, VK_IMAGE_TILING_OPTIMAL, VK_BUFFER_USAGE_TRANSFER_DST_BIT | usageFlags, arrayLayers);
    }
};


USING_PTR (Image2DTransferableLinear);
class GVK_RENDERER_API Image2DTransferableLinear final : public ImageTransferable {
public:
    USING_CREATE (Image2DTransferableLinear);
    Image2DTransferableLinear (const DeviceExtra& device, VkFormat format, uint32_t width, uint32_t height, VkImageUsageFlags usageFlags, uint32_t arrayLayers = 1)
        : ImageTransferable (device, width * height * GetCompontentCountFromFormat (format))
    {
        imageGPU = Image2D::Create (device.GetAllocator (), Image::MemoryLocation::GPU, width, height, format, VK_IMAGE_TILING_LINEAR, VK_BUFFER_USAGE_TRANSFER_DST_BIT | usageFlags, arrayLayers);
    }
};


USING_PTR (Image3DTransferable);
class GVK_RENDERER_API Image3DTransferable final : public ImageTransferable {
public:
    USING_CREATE (Image3DTransferable);
    Image3DTransferable (const DeviceExtra& device, VkFormat format, uint32_t width, uint32_t height, uint32_t depth, VkImageUsageFlags usageFlags)
        : ImageTransferable (device, width * height * depth * GetCompontentCountFromFormat (format))
    {
        imageGPU = Image3D::Create (device.GetAllocator (), Image::MemoryLocation::GPU, width, height, depth, format, VK_IMAGE_TILING_OPTIMAL, VK_BUFFER_USAGE_TRANSFER_DST_BIT | usageFlags);
    }
};


class GVK_RENDERER_API VertexInputInfo final {
public:
    uint32_t                                       size;
    std::vector<VkVertexInputAttributeDescription> attributes;
    std::vector<VkVertexInputBindingDescription>   bindings;

    VertexInputInfo (const std::vector<VkFormat>& vertexInputFormats, VkVertexInputRate inputRate);

    std::vector<VkVertexInputAttributeDescription> GetAttributes (uint32_t nextLocation, uint32_t binding) const;
    std::vector<VkVertexInputBindingDescription>   GetBindings (uint32_t binding) const;
};


USING_PTR (VertexBufferTransferableUntyped);
class GVK_RENDERER_API VertexBufferTransferableUntyped {
public:
    std::vector<uint8_t>     data;
    const VertexInputInfo    info;
    const BufferTransferable buffer;
    const uint32_t           vertexSize;

    USING_CREATE (VertexBufferTransferableUntyped);

    VertexBufferTransferableUntyped (const DeviceExtra& device, uint32_t vertexSize, uint32_t maxVertexCount, const std::vector<VkFormat>& vertexInputFormats, VkVertexInputRate inputRate)
        : info (vertexInputFormats, inputRate)
        , buffer (device, info.size * maxVertexCount, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT)
        , vertexSize (vertexSize)
    {
        data.resize (vertexSize * maxVertexCount);
    }

    void Flush () const
    {
        buffer.CopyAndTransfer (data.data (), data.size ());
    }

    void Bind (VkCommandBuffer commandBuffer) const
    {
        VkBuffer buffers[1] = { buffer.GetBufferToBind () };
        uint64_t offsets[1] = { 0 };
        vkCmdBindVertexBuffers (commandBuffer, 0, 1, buffers, offsets);
    }
};


template<typename VertexType>
class VertexBufferTransferable : public VertexBufferTransferableUntyped {
    USING_CREATE (VertexBufferTransferable);

public:
    VertexBufferTransferable (const DeviceExtra&           device,
                              uint32_t                     maxVertexCount,
                              const std::vector<VkFormat>& vertexInputFormats,
                              VkVertexInputRate            inputRate)
        : VertexBufferTransferableUntyped (device, sizeof (VertexType), maxVertexCount, vertexInputFormats, inputRate)
    {
    }

    void operator= (const std::vector<VertexType>& copiedData)
    {
        const uint32_t copiedBytes = copiedData.size () * sizeof (VertexType);
        GVK_ASSERT (copiedBytes <= data.size ());
        memcpy (data.data (), copiedData.data (), copiedBytes);
    }
};


template<typename IndexType, VkIndexType VulkanIndexType>
class IndexBufferTransferableBase {
    USING_CREATE (IndexBufferTransferableBase);

public:
    std::vector<IndexType>   data;
    const BufferTransferable buffer;

    IndexBufferTransferableBase (const DeviceExtra& device, uint32_t maxIndexCount)
        : buffer (device, sizeof (IndexType) * maxIndexCount, VK_BUFFER_USAGE_INDEX_BUFFER_BIT)
    {
        data.resize (maxIndexCount);
    }

    void Flush () const
    {
        buffer.CopyAndTransfer (data.data (), sizeof (IndexType) * data.size ());
    }

    void operator= (const std::vector<IndexType>& copiedData)
    {
        GVK_ASSERT (copiedData.size () == data.size ());
        memcpy (data.data (), copiedData.data (), copiedData.size () * sizeof (IndexType));
    }

    void Bind (VkCommandBuffer commandBuffer) const
    {
        vkCmdBindIndexBuffer (commandBuffer, buffer.GetBufferToBind (), 0, VulkanIndexType);
    }
};

using IndexBufferTransferable    = IndexBufferTransferableBase<uint16_t, VK_INDEX_TYPE_UINT16>;
using IndexBufferTransferableU   = U<IndexBufferTransferableBase<uint16_t, VK_INDEX_TYPE_UINT16>>;
using IndexBufferTransferable32  = IndexBufferTransferableBase<uint32_t, VK_INDEX_TYPE_UINT32>;
using IndexBufferTransferable32U = U<IndexBufferTransferableBase<uint32_t, VK_INDEX_TYPE_UINT32>>;


#endif