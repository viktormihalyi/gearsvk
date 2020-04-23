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

void TransitionImageLayout (VkDevice device, VkQueue queue, VkCommandPool commandPool, const ImageBase& image, VkImageLayout oldLayout, VkImageLayout newLayout);

void CopyBufferToImage (VkCommandBuffer commandBuffer, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t depth = 1);
void CopyBufferToImage (VkDevice device, VkQueue graphicsQueue, VkCommandPool commandPool, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t depth = 1);

void CopyBuffer (VkDevice device, VkQueue graphicsQueue, VkCommandPool commandPool, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

std::vector<uint8_t> ReadImage (const std::filesystem::path& filePath, uint32_t components = 4);

bool AreImagesEqual (const Device& device, VkQueue queue, VkCommandPool commandPool, const Image2D& image, const std::filesystem::path& expectedImage);

std::thread SaveImageToFileAsync (const Device& device, VkQueue queue, VkCommandPool commandPool, const Image2D& image, const std::filesystem::path& filePath);


struct AllocatedImage final {
    ImageBase::U    image;
    DeviceMemory::U memory;

    USING_PTR (AllocatedImage);

    AllocatedImage (const Device& device, ImageBase::U&& image, VkMemoryPropertyFlags memoryPropertyFlags)
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


#endif