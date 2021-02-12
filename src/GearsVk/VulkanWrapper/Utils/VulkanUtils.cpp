#include "VulkanUtils.hpp"

#include "stb_image.h"
#include "stb_image_write.h"

#include "ImageData.hpp"

#include <array>
#include <cstring>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>

namespace GVK {

std::string GetVersionString (uint32_t version)
{
    std::stringstream ss;
    ss << VK_VERSION_MAJOR (version) << "."
       << VK_VERSION_MINOR (version) << "."
       << VK_VERSION_PATCH (version);
    return ss.str ();
}


void TransitionImageLayout (const DeviceExtra& device, const Image& image, VkImageLayout oldLayout, VkImageLayout newLayout)
{
    SingleTimeCommand commandBuffer (device);
    image.CmdPipelineBarrier (commandBuffer, oldLayout, newLayout);
}


void CopyBufferToImage (const DeviceExtra& device, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t depth)
{
    SingleTimeCommand commandBuffer (device);
    CopyBufferToImage (commandBuffer, buffer, image, width, height, depth);
}


void CopyBufferToImage (CommandBuffer& commandBuffer, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t depth)
{
    VkBufferImageCopy region               = {};
    region.bufferOffset                    = 0;
    region.bufferRowLength                 = 0;
    region.bufferImageHeight               = 0;
    region.imageSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel       = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount     = 1;
    region.imageOffset                     = { 0, 0, 0 };
    region.imageExtent                     = { width, height, depth };

    commandBuffer.RecordT<CommandGeneric> ([&] (VkCommandBuffer commandBuffer) {
        vkCmdCopyBufferToImage (
            commandBuffer,
            buffer,
            image,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1,
            &region);
    });
}


void CopyBuffer (const DeviceExtra& device, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
    SingleTimeCommand commandBuffer (device);

    VkBufferCopy copyRegion = {};
    copyRegion.srcOffset    = 0;
    copyRegion.dstOffset    = 0;
    copyRegion.size         = size;

    commandBuffer.RecordT<CommandGeneric> ([&] (VkCommandBuffer commandBuffer) {
        vkCmdCopyBuffer (commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
    });
}


static ImageU CreateCopyImageOnCPU (const DeviceExtra& device, const Image& image, uint32_t layerIndex = 0)
{
    const uint32_t width  = image.GetWidth ();
    const uint32_t height = image.GetHeight ();

    Image2DU dst = Make<Image2D> (device.GetAllocator (), Image::MemoryLocation::CPU, image.GetWidth (), image.GetHeight (), image.GetFormat (), VK_IMAGE_TILING_LINEAR, VK_IMAGE_USAGE_TRANSFER_DST_BIT, 1);

    TransitionImageLayout (device, *dst, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    {
        SingleTimeCommand single (device);

        VkImageCopy imageCopyRegion                   = {};
        imageCopyRegion.srcSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        imageCopyRegion.srcSubresource.layerCount     = 1;
        imageCopyRegion.srcSubresource.baseArrayLayer = layerIndex;
        imageCopyRegion.dstSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        imageCopyRegion.dstSubresource.layerCount     = 1;
        imageCopyRegion.extent.width                  = image.GetWidth ();
        imageCopyRegion.extent.height                 = image.GetHeight ();
        imageCopyRegion.extent.depth                  = 1;

        single.RecordT<CommandGeneric> ([&] (VkCommandBuffer commandBuffer) {
            vkCmdCopyImage (
                commandBuffer,
                image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                *dst, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                1,
                &imageCopyRegion);
        });
    }

    return dst;
}


std::vector<uint8_t> ReadImage (const std::filesystem::path& filePath, uint32_t components)
{
    int width, height, readComponents;

    unsigned char* imageData = stbi_load (filePath.u8string ().c_str (), &width, &height, &readComponents, components);

    if (GVK_ERROR (imageData == nullptr)) {
        throw std::runtime_error ("failed to load image");
    }

    std::vector<uint8_t> imageBytes (width * height * components);
    memcpy (imageBytes.data (), imageData, width * height * components);

    stbi_image_free (imageData);

    return imageBytes;
}

// copy image to cpu and compare with a reference
bool AreImagesEqual (const DeviceExtra& device, const Image& image, const std::filesystem::path& expectedImage, uint32_t layerIndex)
{
    const uint32_t width      = image.GetWidth ();
    const uint32_t height     = image.GetHeight ();
    const uint32_t pixelCount = width * height;
    const uint32_t byteCount  = pixelCount * 4;

    ImageU dst = CreateCopyImageOnCPU (device, image, layerIndex);


    std::vector<std::array<uint8_t, 4>> mapped (pixelCount);

    {
        MemoryMapping mapping (device.GetAllocator (), *dst);
        memcpy (mapped.data (), mapping.Get (), byteCount);
    }

    std::vector<std::array<uint8_t, 4>> expected (pixelCount);
    int                                 expectedWidth, expectedHeight, expectedComponents;
    unsigned char*                      exepctedData = stbi_load (expectedImage.u8string ().c_str (), &expectedWidth, &expectedHeight, &expectedComponents, STBI_rgb_alpha);
    if (GVK_ERROR (expectedWidth != width || expectedHeight != height || expectedComponents != 4)) {
        stbi_image_free (exepctedData);
        return false;
    }

    memcpy (expected.data (), exepctedData, byteCount);
    stbi_image_free (exepctedData);

    return memcmp (expected.data (), mapped.data (), byteCount) == 0;
}


std::thread SaveImageToFileAsync (const DeviceExtra& device, const Image& image, const std::filesystem::path& filePath, uint32_t layerIndex)
{
    return std::thread ([=, &image] () {
        ImageData (device, image, layerIndex).SaveTo (filePath);
    });
};

}