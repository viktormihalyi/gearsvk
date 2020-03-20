#include "VulkanUtils.hpp"

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "stb_image.h"
#include "stb_image_write.h"

#include <array>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>


std::string GetVersionString (uint32_t version)
{
    std::stringstream ss;
    ss << VK_VERSION_MAJOR (version) << "."
       << VK_VERSION_MINOR (version) << "."
       << VK_VERSION_PATCH (version);
    return ss.str ();
}


void TransitionImageLayout (VkDevice device, VkQueue queue, VkCommandPool commandPool, const Image& image, VkImageLayout oldLayout, VkImageLayout newLayout)
{
    SingleTimeCommand commandBuffer (device, commandPool, queue);
    image.CmdTransitionImageLayout (commandBuffer, oldLayout, newLayout);
}


void CopyBufferToImage (VkDevice device, VkQueue graphicsQueue, VkCommandPool commandPool, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
{
    SingleTimeCommand commandBuffer (device, commandPool, graphicsQueue);

    VkBufferImageCopy region               = {};
    region.bufferOffset                    = 0;
    region.bufferRowLength                 = 0;
    region.bufferImageHeight               = 0;
    region.imageSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel       = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount     = 1;
    region.imageOffset                     = {0, 0, 0};
    region.imageExtent                     = {width, height, 1};

    vkCmdCopyBufferToImage (
        commandBuffer,
        buffer,
        image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1,
        &region);
}


AllocatedImage CreateImage (const Device& device, uint32_t width, uint32_t height, VkQueue queue, VkCommandPool commandPool)
{
    AllocatedImage result (device, Image::Create (device, width, height, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT), DeviceMemory::GPU);

    TransitionImageLayout (device, queue, commandPool, *result.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    {
        AllocatedBuffer stagingMemory (device, Buffer::Create (device, width * height * 4, VK_BUFFER_USAGE_TRANSFER_SRC_BIT), DeviceMemory::CPU);
        {
            MemoryMapping                       bm (device, *stagingMemory.memory, 0, width * height * 4);
            std::vector<std::array<uint8_t, 4>> pixels (width * height);
            for (uint32_t y = 0; y < height; ++y) {
                for (uint32_t x = 0; x < width; ++x) {
                    pixels[y * width + x] = {1, 1, 127, 127};
                }
            }

            bm.Copy (pixels);
        }
        CopyBufferToImage (device, queue, commandPool, *stagingMemory.buffer, *result.image, width, height);
    }
    TransitionImageLayout (device, queue, commandPool, *result.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);


    return std::move (result);
}


static AllocatedImage CreateCopyImageOnCPU (const Device& device, VkQueue queue, VkCommandPool commandPool, const Image& image)
{
    const uint32_t width  = image.GetWidth ();
    const uint32_t height = image.GetHeight ();

    AllocatedImage dst (device, Image::Create (device, image.GetWidth (), image.GetHeight (), image.GetFormat (), VK_IMAGE_TILING_LINEAR, VK_IMAGE_USAGE_TRANSFER_DST_BIT), DeviceMemory::CPU);
    TransitionImageLayout (device, queue, commandPool, *dst.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    {
        SingleTimeCommand single (device, commandPool, queue);

        VkImageCopy imageCopyRegion               = {};
        imageCopyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageCopyRegion.srcSubresource.layerCount = 1;
        imageCopyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageCopyRegion.dstSubresource.layerCount = 1;
        imageCopyRegion.extent.width              = image.GetWidth ();
        imageCopyRegion.extent.height             = image.GetHeight ();
        imageCopyRegion.extent.depth              = 1;

        vkCmdCopyImage (
            single,
            image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            *dst.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1,
            &imageCopyRegion);
    }

    return dst;
}


// copy image to cpu and compare with a reference
bool AreImagesEqual (const Device& device, VkQueue queue, VkCommandPool commandPool, const Image& image, const std::filesystem::path& expectedImage)
{
    const uint32_t width  = image.GetWidth ();
    const uint32_t height = image.GetHeight ();

    AllocatedImage dst = CreateCopyImageOnCPU (device, queue, commandPool, image);


    std::vector<std::array<uint8_t, 4>> mapped (width * height);

    {
        MemoryMapping mapping (device, *dst.memory, 0, width * height * 4);
        std::memcpy (mapped.data (), mapping.Get (), width * height * 4);
    }

    std::vector<std::array<uint8_t, 4>> expected (width * height);
    int                                 expectedWidth, expectedHeight, expectedComponents;
    unsigned char*                      exepctedData = stbi_load (expectedImage.u8string ().c_str (), &expectedWidth, &expectedHeight, &expectedComponents, STBI_rgb_alpha);
    if (ERROR (expectedWidth != width || expectedHeight != height || expectedComponents != 4)) {
        stbi_image_free (exepctedData);
        return false;
    }

    std::memcpy (expected.data (), exepctedData, width * height * 4);
    stbi_image_free (exepctedData);

    return std::memcmp (expected.data (), mapped.data (), width * height * 4) == 0;
}


std::thread SaveImageToFileAsync (const Device& device, VkQueue queue, VkCommandPool commandPool, const Image& image, const std::filesystem::path& filePath)
{
    std::cout << "saving an image to" << filePath << std::endl;

    const uint32_t width  = image.GetWidth ();
    const uint32_t height = image.GetHeight ();

    AllocatedImage dst = CreateCopyImageOnCPU (device, queue, commandPool, image);

    std::vector<std::array<uint8_t, 4>> mapped (width * height);

    {
        MemoryMapping mapping (device, *dst.memory, 0, width * height * 4);
        std::memcpy (mapped.data (), mapping.Get (), width * height * 4);
    }

    return std::thread ([=] () {
        stbi_write_png (filePath.u8string ().c_str (), width, height, 4, mapped.data (), width * 4);
        std::cout << "writing png done" << std::endl;
    });
};
