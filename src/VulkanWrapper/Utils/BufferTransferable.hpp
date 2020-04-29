#ifndef BUFFER_TRANSFERABLE_HPP
#define BUFFER_TRANSFERABLE_HPP

#include "Buffer.hpp"
#include "CommandBuffer.hpp"
#include "Device.hpp"
#include "DeviceMemory.hpp"
#include "Image.hpp"
#include "MemoryMapping.hpp"
#include "ShaderSourceBuilder.hpp"
#include "SingleTimeCommand.hpp"
#include "VulkanUtils.hpp"

#include <cstring>

class ITransferableBuffer {
public:
    virtual void     CopyAndTransfer (const void* data, size_t size) const = 0;
    virtual VkBuffer GetBufferToBind () const                              = 0;
};

class ITransferableImage {
public:
    virtual void    CopyTransitionTransfer (VkImageLayout currentImageLayout, const void* data, size_t size, std::optional<VkImageLayout> nextLayout = std::nullopt) const = 0;
    virtual VkImage GetImageToBind () const                                                                                                                                = 0;
};

class BufferTransferable final {
public:
    const VkDevice      device;
    const VkQueue       queue;
    const VkCommandPool commandPool;

    uint32_t bufferSize;

    AllocatedBuffer bufferGPU;

    AllocatedBuffer bufferCPU;
    MemoryMapping   bufferCPUMapping;

    USING_PTR (BufferTransferable);

    BufferTransferable (const Device& device, VkQueue queue, VkCommandPool commandPool, uint32_t bufferSize, VkBufferUsageFlags usageFlags)
        : device (device)
        , queue (queue)
        , commandPool (commandPool)
        , bufferSize (bufferSize)
        , bufferGPU (device, Buffer::Create (device, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | usageFlags), DeviceMemory::GPU)
        , bufferCPU (device, Buffer::Create (device, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT), DeviceMemory::CPU)
        , bufferCPUMapping (device, *bufferCPU.memory, 0, bufferSize)
    {
    }

    void CopyAndTransfer (const void* data, size_t size) const
    {
        ASSERT (size == bufferSize);
        bufferCPUMapping.Copy (data, size, 0);
        CopyBuffer (device, queue, commandPool, *bufferCPU.buffer, *bufferGPU.buffer, bufferSize);
    }

    VkBuffer GetBufferToBind () const
    {
        return *bufferGPU.buffer;
    }
};

class ImageTransferableBase {
private:
    const VkDevice      device;
    const VkQueue       queue;
    const VkCommandPool commandPool;

    uint32_t bufferSize;

    AllocatedBuffer bufferCPU;
    MemoryMapping   bufferCPUMapping;

public:
    AllocatedImage::U imageGPU;

    USING_PTR (ImageTransferableBase);

protected:
    ImageTransferableBase (const Device& device, VkQueue queue, VkCommandPool commandPool, uint32_t bufferSize)
        : device (device)
        , queue (queue)
        , commandPool (commandPool)
        , bufferSize (bufferSize)
        , bufferCPU (device, Buffer::Create (device, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT), DeviceMemory::CPU)
        , bufferCPUMapping (device, *bufferCPU.memory, 0, bufferSize)
    {
    }

public:
    virtual ~ImageTransferableBase () = default;

    void CopyTransitionTransfer (VkImageLayout currentImageLayout, const void* data, size_t size, std::optional<VkImageLayout> nextLayout = std::nullopt) const
    {
        ASSERT (size == bufferSize);

        bufferCPUMapping.Copy (data, size, 0);

        SingleTimeCommand commandBuffer (device, commandPool, queue);

        imageGPU->image->CmdPipelineBarrier (commandBuffer, currentImageLayout, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        imageGPU->image->CmdCopyBufferToImage (commandBuffer, *bufferCPU.buffer);
        if (nextLayout.has_value ()) {
            imageGPU->image->CmdPipelineBarrier (commandBuffer, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, *nextLayout);
        }
    }

    VkImage GetImageToBind () const
    {
        return *imageGPU->image;
    }
};


static uint32_t GetCompontentCountFromFormat (VkFormat format)
{
    switch (format) {
        case VK_FORMAT_R8_UINT: return 1;
        case VK_FORMAT_R8G8_UINT: return 2;
        case VK_FORMAT_R8G8B8_UINT: return 3;
        case VK_FORMAT_R8G8B8A8_UINT: return 4;
        case VK_FORMAT_R8_SRGB: return 1;
        case VK_FORMAT_R8G8_SRGB: return 2;
        case VK_FORMAT_R8G8B8_SRGB: return 3;
        case VK_FORMAT_R8G8B8A8_SRGB: return 4;
        default:
            ASSERT (false);
            return 4;
    }
}


class Image1DTransferable final : public ImageTransferableBase {
public:
    USING_PTR (Image1DTransferable);
    Image1DTransferable (const Device& device, VkQueue queue, VkCommandPool commandPool, VkFormat format, uint32_t width, VkImageUsageFlags usageFlags)
        : ImageTransferableBase (device, queue, commandPool, width * GetCompontentCountFromFormat (format))
    {
        imageGPU = AllocatedImage::Create (device, Image1D::Create (device, width, format, VK_IMAGE_TILING_OPTIMAL, VK_BUFFER_USAGE_TRANSFER_DST_BIT | usageFlags), DeviceMemory::GPU);
    }
};


class Image2DTransferable final : public ImageTransferableBase {
public:
    USING_PTR (Image2DTransferable);
    Image2DTransferable (const Device& device, VkQueue queue, VkCommandPool commandPool, VkFormat format, uint32_t width, uint32_t height, VkImageUsageFlags usageFlags)
        : ImageTransferableBase (device, queue, commandPool, width * height * GetCompontentCountFromFormat (format))
    {
        imageGPU = AllocatedImage::Create (device, Image2D::Create (device, width, height, format, VK_IMAGE_TILING_OPTIMAL, VK_BUFFER_USAGE_TRANSFER_DST_BIT | usageFlags), DeviceMemory::GPU);
    }
};


class Image3DTransferable final : public ImageTransferableBase {
public:
    USING_PTR (Image3DTransferable);
    Image3DTransferable (const Device& device, VkQueue queue, VkCommandPool commandPool, VkFormat format, uint32_t width, uint32_t height, uint32_t depth, VkImageUsageFlags usageFlags)
        : ImageTransferableBase (device, queue, commandPool, width * height * depth * GetCompontentCountFromFormat (format))
    {
        imageGPU = AllocatedImage::Create (device, Image3D::Create (device, width, height, depth, format, VK_IMAGE_TILING_OPTIMAL, VK_BUFFER_USAGE_TRANSFER_DST_BIT | usageFlags), DeviceMemory::GPU);
    }
};


class VertexInputInfo final : public ShaderSourceBuilder {
public:
    uint32_t                                       size;
    std::vector<VkVertexInputAttributeDescription> attributes;
    std::vector<VkVertexInputBindingDescription>   bindings;
    std::optional<std::vector<std::string>>        attributeNames;

    VertexInputInfo (const std::vector<VkFormat>& vertexInputFormats, const std::optional<std::vector<std::string>>& attributeNames = std::nullopt);

    std::string GetProvidedShaderSource () const override;
};


class VertexBufferTransferableUntyped {
public:
    std::vector<uint8_t>     data;
    const VertexInputInfo    info;
    const BufferTransferable buffer;
    const uint32_t           vertexSize;

    USING_PTR (VertexBufferTransferableUntyped);

    VertexBufferTransferableUntyped (const Device& device, VkQueue queue, VkCommandPool commandPool, uint32_t vertexSize, uint32_t maxVertexCount, const std::vector<VkFormat>& vertexInputFormats,
                                     const std::optional<std::vector<std::string>>& attributeNames = std::nullopt)
        : info (vertexInputFormats, attributeNames)
        , buffer (device, queue, commandPool, info.size * maxVertexCount, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT)
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
        VkBuffer buffers[1] = {buffer.GetBufferToBind ()};
        uint64_t offsets[1] = {0};
        vkCmdBindVertexBuffers (commandBuffer, 0, 1, buffers, offsets);
    }
};


template<typename VertexType>
class VertexBufferTransferable : public VertexBufferTransferableUntyped {
public:
    USING_PTR (VertexBufferTransferable);

    VertexBufferTransferable (const Device& device, VkQueue queue, VkCommandPool commandPool,
                              uint32_t                                       maxVertexCount,
                              const std::vector<VkFormat>&                   vertexInputFormats,
                              const std::optional<std::vector<std::string>>& attributeNames = std::nullopt)
        : VertexBufferTransferableUntyped (device, queue, commandPool, sizeof (VertexType), maxVertexCount, vertexInputFormats, attributeNames)
    {
    }

    void operator= (const std::vector<VertexType>& copiedData)
    {
        const uint32_t copiedBytes = copiedData.size () * sizeof (VertexType);
        ASSERT (copiedBytes <= data.size ());
        memcpy (data.data (), copiedData.data (), copiedBytes);
    }
};


class IndexBufferTransferable {
public:
    using IndexType = uint16_t;

    std::vector<IndexType>   data;
    const BufferTransferable buffer;

    USING_PTR (IndexBufferTransferable);

    IndexBufferTransferable (const Device& device, VkQueue queue, VkCommandPool commandPool, uint32_t maxIndexCount)
        : buffer (device, queue, commandPool, sizeof (IndexType) * maxIndexCount, VK_BUFFER_USAGE_INDEX_BUFFER_BIT)
    {
        data.resize (maxIndexCount);
    }

    void Flush () const
    {
        buffer.CopyAndTransfer (data.data (), sizeof (IndexType) * data.size ());
    }

    void operator= (const std::vector<uint16_t>& copiedData)
    {
        ASSERT (copiedData.size () == data.size ());
        memcpy (data.data (), copiedData.data (), copiedData.size () * sizeof (IndexType));
    }

    void Bind (VkCommandBuffer commandBuffer) const
    {
        vkCmdBindIndexBuffer (commandBuffer, buffer.GetBufferToBind (), 0, VK_INDEX_TYPE_UINT16);
    }
};


#endif