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

struct AllocatedImage {
    Image::U        image;
    DeviceMemory::U memory;

    AllocatedImage (const Device& device, Image::U&& image, VkMemoryPropertyFlags memoryPropertyFlags)
        : image (std::move (image))
        , memory (DeviceMemory::Create (device, device.GetImageAllocateInfo (*this->image, memoryPropertyFlags)))
    {
        vkBindImageMemory (device, *this->image, *memory, 0);
    }
};


struct AllocatedBuffer {
    Buffer::U       buffer;
    DeviceMemory::U memory;

    AllocatedBuffer (const Device& device, Buffer::U&& buffer, VkMemoryPropertyFlags memoryPropertyFlags)
        : buffer (std::move (buffer))
        , memory (DeviceMemory::Create (device, device.GetBufferAllocateInfo (*this->buffer, memoryPropertyFlags)))
    {
        vkBindBufferMemory (device, *this->buffer, *memory, 0);
    }
};


void TransitionImageLayout (VkDevice device, VkQueue queue, VkCommandPool commandPool, const Image& image, VkImageLayout oldLayout, VkImageLayout newLayout);

void CopyBufferToImage (VkDevice device, VkQueue graphicsQueue, VkCommandPool commandPool, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

AllocatedImage CreateImage (const Device& device, uint32_t width, uint32_t height, VkQueue queue, VkCommandPool commandPool);

bool AreImagesEqual (const Device& device, VkQueue queue, VkCommandPool commandPool, const Image& image, const std::filesystem::path& expectedImage);

std::thread SaveImageToFileAsync (const Device& device, VkQueue queue, VkCommandPool commandPool, const Image& image, const std::filesystem::path& filePath);


#endif