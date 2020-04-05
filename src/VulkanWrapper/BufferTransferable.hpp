#ifndef BUFFER_TRANSFERABLE_HPP
#define BUFFER_TRANSFERABLE_HPP

#include "Buffer.hpp"
#include "CommandBuffer.hpp"
#include "Device.hpp"
#include "DeviceMemory.hpp"
#include "Image.hpp"
#include "MemoryMapping.hpp"
#include "SingleTimeCommand.hpp"
#include "VulkanUtils.hpp"


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

    void CopyAndTransfer (const void* data, uint32_t size) const
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


class VertexInputInfo final {
public:
    uint32_t                                       size;
    std::vector<VkVertexInputAttributeDescription> attributes;
    std::vector<VkVertexInputBindingDescription>   bindings;

    VertexInputInfo (const std::vector<VkFormat>& vertexInputFormats);
};


class VertexBufferTransferableUntyped {
public:
    std::vector<uint8_t>     data;
    const VertexInputInfo    info;
    const BufferTransferable buffer;
    const uint32_t           vertexSize;

    USING_PTR (VertexBufferTransferableUntyped);

    VertexBufferTransferableUntyped (const Device& device, VkQueue queue, VkCommandPool commandPool, uint32_t vertexSize, const std::vector<VkFormat>& vertexInputFormats, uint32_t maxVertexCount)
        : info (vertexInputFormats)
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

    VertexBufferTransferable (const Device& device, VkQueue queue, VkCommandPool commandPool, const std::vector<VkFormat>& vertexInputFormats, uint32_t maxVertexCount)
        : VertexBufferTransferableUntyped (device, queue, commandPool, sizeof (VertexType), vertexInputFormats, maxVertexCount)
    {
    }

    void operator= (const std::vector<VertexType>& copiedData)
    {
        const uint32_t copiedBytes = copiedData.size () * sizeof (VertexType);
        ASSERT (copiedBytes <= data.size ());
        std::memcpy (data.data (), copiedData.data (), copiedBytes);
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
        std::memcpy (data.data (), copiedData.data (), copiedData.size () * sizeof (IndexType));
    }

    void Bind (VkCommandBuffer commandBuffer) const
    {
        vkCmdBindIndexBuffer (commandBuffer, buffer.GetBufferToBind (), 0, VK_INDEX_TYPE_UINT16);
    }
};

#endif