#ifndef VULKANWRAPPER_VULKANUTILS_HPP
#define VULKANWRAPPER_VULKANUTILS_HPP

#include "Buffer.hpp"
#include "CommandBuffer.hpp"
#include "Device.hpp"
#include "DeviceMemory.hpp"
#include "Image.hpp"
#include "MemoryMapping.hpp"
#include "SingleTimeCommand.hpp"


#include <array>
#include <filesystem>
#include <thread>


std::string GetVersionString (uint32_t version);

void TransitionImageLayout (VkDevice device, VkQueue queue, VkCommandPool commandPool, const Image& image, VkImageLayout oldLayout, VkImageLayout newLayout);

void CopyBufferToImage (VkDevice device, VkQueue graphicsQueue, VkCommandPool commandPool, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

void CopyBuffer (VkDevice device, VkQueue graphicsQueue, VkCommandPool commandPool, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

bool AreImagesEqual (const Device& device, VkQueue queue, VkCommandPool commandPool, const Image& image, const std::filesystem::path& expectedImage);

std::thread SaveImageToFileAsync (const Device& device, VkQueue queue, VkCommandPool commandPool, const Image& image, const std::filesystem::path& filePath);


struct AllocatedImage final {
    Image::U        image;
    DeviceMemory::U memory;

    USING_PTR (AllocatedImage);

    AllocatedImage (const Device& device, Image::U&& image, VkMemoryPropertyFlags memoryPropertyFlags)
        : image (std::move (image))
        , memory (DeviceMemory::Create (device, device.GetImageAllocateInfo (*this->image, memoryPropertyFlags)))
    {
        vkBindImageMemory (device, *this->image, *memory, 0);
    }

    static AllocatedImage CreatePreinitialized (const Device& device, uint32_t width, uint32_t height, VkQueue queue, VkCommandPool commandPool);
};


struct AllocatedBuffer final {
    Buffer::U       buffer;
    DeviceMemory::U memory;

    USING_PTR (AllocatedBuffer);

    AllocatedBuffer (const Device& device, Buffer::U&& buffer, VkMemoryPropertyFlags memoryPropertyFlags)
        : buffer (std::move (buffer))
        , memory (DeviceMemory::Create (device, device.GetBufferAllocateInfo (*this->buffer, memoryPropertyFlags)))
    {
        vkBindBufferMemory (device, *this->buffer, *memory, 0);
    }
};

namespace ShaderTypes {

static constexpr VkFormat Float = VK_FORMAT_R32_SFLOAT;
static constexpr VkFormat Vec1f = VK_FORMAT_R32_SFLOAT;
static constexpr VkFormat Vec2f = VK_FORMAT_R32G32_SFLOAT;
static constexpr VkFormat Vec3f = VK_FORMAT_R32G32B32_SFLOAT;
static constexpr VkFormat Vec4f = VK_FORMAT_R32G32B32A32_SFLOAT;

static constexpr VkFormat Uint  = VK_FORMAT_R32_UINT;
static constexpr VkFormat Vec1u = VK_FORMAT_R32_UINT;
static constexpr VkFormat Vec2u = VK_FORMAT_R32G32_UINT;
static constexpr VkFormat Vec3u = VK_FORMAT_R32G32B32_UINT;
static constexpr VkFormat Vec4u = VK_FORMAT_R32G32B32A32_UINT;

}; // namespace ShaderTypes

class VertexInputInfo final {
public:
    uint32_t                                       size;
    std::vector<VkVertexInputAttributeDescription> attributes;
    std::vector<VkVertexInputBindingDescription>   bindings;

    VertexInputInfo (const std::vector<VkFormat>& vertexInputFormats);
};


class TransferableBuffer final {
public:
    const VkDevice      device;
    const VkQueue       queue;
    const VkCommandPool commandPool;

    uint32_t bufferSize;

    AllocatedBuffer bufferGPU;

    AllocatedBuffer bufferCPU;
    MemoryMapping   bufferCPUMapping;

    USING_PTR (TransferableBuffer);

    TransferableBuffer (const Device& device, VkQueue queue, VkCommandPool commandPool, uint32_t bufferSize, VkBufferUsageFlags usageFlags)
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


class UntypedTransferableVertexBuffer {
public:
    std::vector<uint8_t>     data;
    const VertexInputInfo    info;
    const TransferableBuffer buffer;
    const uint32_t           vertexSize;

    USING_PTR (UntypedTransferableVertexBuffer);

    UntypedTransferableVertexBuffer (const Device& device, VkQueue queue, VkCommandPool commandPool, uint32_t vertexSize, const std::vector<VkFormat>& vertexInputFormats, uint32_t maxVertexCount)
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
};

template<typename VertexType>
class TypedTransferableVertexBuffer : public UntypedTransferableVertexBuffer {
public:
    USING_PTR (TypedTransferableVertexBuffer);

    TypedTransferableVertexBuffer (const Device& device, VkQueue queue, VkCommandPool commandPool, const std::vector<VkFormat>& vertexInputFormats, uint32_t maxVertexCount)
        : UntypedTransferableVertexBuffer (device, queue, commandPool, sizeof (VertexType), vertexInputFormats, maxVertexCount)
    {
    }

    void operator= (const std::vector<VertexType>& copiedData)
    {
        const uint32_t copiedBytes = copiedData.size () * sizeof (VertexType);
        ASSERT (copiedBytes <= data.size ());
        std::memcpy (data.data (), copiedData.data (), copiedBytes);
    }
};


class TransferableIndexBuffer {
public:
    std::vector<uint16_t>    data;
    const TransferableBuffer buffer;

    USING_PTR (TransferableIndexBuffer);

    TransferableIndexBuffer (const Device& device, VkQueue queue, VkCommandPool commandPool, uint32_t maxIndexCount)
        : buffer (device, queue, commandPool, sizeof (uint16_t) * maxIndexCount, VK_BUFFER_USAGE_INDEX_BUFFER_BIT)
    {
        data.resize (maxIndexCount);
    }

    void Flush () const
    {
        buffer.CopyAndTransfer (data.data (), sizeof (uint16_t) * data.size ());
    }
};


#endif