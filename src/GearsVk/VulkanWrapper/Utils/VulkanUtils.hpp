#ifndef VULKANWRAPPER_VULKANUTILS_HPP
#define VULKANWRAPPER_VULKANUTILS_HPP

#include "GearsVkAPI.hpp"

#include "Buffer.hpp"
#include "CommandBuffer.hpp"
#include "Device.hpp"
#include "DeviceExtra.hpp"
#include "DeviceMemory.hpp"
#include "Image.hpp"
#include "MemoryMapping.hpp"
#include "SingleTimeCommand.hpp"


#include <array>
#include <filesystem>
#include <thread>


std::string GetVersionString (uint32_t version);

GEARSVK_API
void TransitionImageLayout (const DeviceExtra& device, const ImageBase& image, VkImageLayout oldLayout, VkImageLayout newLayout);

void CopyBufferToImage (VkCommandBuffer commandBuffer, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t depth = 1);
void CopyBufferToImage (const DeviceExtra& device, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t depth = 1);

GEARSVK_API
void CopyBuffer (const DeviceExtra& device, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

GEARSVK_API
bool AreImagesEqual (const DeviceExtra& device, const ImageBase& image, const std::filesystem::path& expectedImage, uint32_t layerIndex = 0);

GEARSVK_API
std::thread SaveImageToFileAsync (const DeviceExtra& device, const ImageBase& image, const std::filesystem::path& filePath, uint32_t layerIndex = 0);


USING_PTR (AllocatedImage);
struct GEARSVK_API AllocatedImage final {
    ImageBaseU    image;
    DeviceMemoryU memory;

    USING_CREATE (AllocatedImage);

    AllocatedImage (const DeviceExtra& device, ImageBaseU&& image, VkMemoryPropertyFlags memoryPropertyFlags)
        : image (std::move (image))
        , memory (DeviceMemory::Create (device, device.GetImageAllocateInfo (*this->image, memoryPropertyFlags)))
    {
        vkBindImageMemory (device, *this->image, *memory, 0);
    }

    static AllocatedImage CreatePreinitialized (const DeviceExtra& device, uint32_t width, uint32_t height);
};


USING_PTR (AllocatedBuffer);
struct GEARSVK_API AllocatedBuffer final {
    BufferU       buffer;
    DeviceMemoryU memory;

    USING_CREATE (AllocatedBuffer);

    AllocatedBuffer (const Device& device, BufferU&& buffer, VkMemoryPropertyFlags memoryPropertyFlags)
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