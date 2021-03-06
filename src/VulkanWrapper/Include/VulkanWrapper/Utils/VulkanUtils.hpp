#ifndef VULKANWRAPPER_VULKANUTILS_HPP
#define VULKANWRAPPER_VULKANUTILS_HPP

#include "VulkanWrapper/VulkanWrapperAPI.hpp"

#include "VulkanWrapper/Buffer.hpp"
#include "VulkanWrapper/CommandBuffer.hpp"
#include "VulkanWrapper/Device.hpp"
#include "VulkanWrapper/DeviceExtra.hpp"
#include "VulkanWrapper/DeviceMemory.hpp"
#include "VulkanWrapper/Image.hpp"
#include "VulkanWrapper/Utils/MemoryMapping.hpp"
#include "VulkanWrapper/Utils/SingleTimeCommand.hpp"


#include <array>
#include <filesystem>
#include <thread>

namespace GVK {

VULKANWRAPPER_API
std::string GetVersionString (uint32_t version);

VULKANWRAPPER_API
void TransitionImageLayout (const DeviceExtra& device, const Image& image, VkImageLayout oldLayout, VkImageLayout newLayout);

VULKANWRAPPER_API
void CopyBufferToImage (CommandBuffer& commandBuffer, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t depth = 1);

VULKANWRAPPER_API
void CopyBufferToImage (const DeviceExtra& device, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t depth = 1);

VULKANWRAPPER_API
void CopyBuffer (const DeviceExtra& device, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

VULKANWRAPPER_API
bool AreImagesEqual (const DeviceExtra& device, const Image& image, const std::filesystem::path& expectedImage, uint32_t layerIndex = 0);

VULKANWRAPPER_API
std::thread SaveImageToFileAsync (const DeviceExtra& device, const Image& image, const std::filesystem::path& filePath, uint32_t layerIndex = 0);

VULKANWRAPPER_API
uint32_t GetCompontentCountFromFormat (VkFormat format);

VULKANWRAPPER_API
uint32_t GetEachCompontentSizeFromFormat (VkFormat format);


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

}

#endif