#include "VulkanUtils.hpp"

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "stb_image.h"
#include "stb_image_write.h"

#include <array>
#include <cstring>
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


void TransitionImageLayout (const DeviceExtra& device, const ImageBase& image, VkImageLayout oldLayout, VkImageLayout newLayout)
{
    SingleTimeCommand commandBuffer (device);
    image.CmdPipelineBarrier (commandBuffer, oldLayout, newLayout);
}


void CopyBufferToImage (const DeviceExtra& device, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t depth)
{
    SingleTimeCommand commandBuffer (device);
    CopyBufferToImage (commandBuffer, buffer, image, width, height, depth);
}


void CopyBufferToImage (VkCommandBuffer commandBuffer, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t depth)
{
    VkBufferImageCopy region               = {};
    region.bufferOffset                    = 0;
    region.bufferRowLength                 = 0;
    region.bufferImageHeight               = 0;
    region.imageSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel       = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount     = 1;
    region.imageOffset                     = {0, 0, 0};
    region.imageExtent                     = {width, height, depth};

    vkCmdCopyBufferToImage (
        commandBuffer,
        buffer,
        image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1,
        &region);
}


void CopyBuffer (const DeviceExtra& device, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
    SingleTimeCommand commandBuffer (device);

    VkBufferCopy copyRegion = {};
    copyRegion.size         = size;
    vkCmdCopyBuffer (commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
}


AllocatedImage AllocatedImage::CreatePreinitialized (const DeviceExtra& device, uint32_t width, uint32_t height)
{
    AllocatedImage result (device, Image2D::Create (device, width, height, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 1), DeviceMemory::GPU);

    TransitionImageLayout (device, *result.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
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
        CopyBufferToImage (device, *stagingMemory.buffer, *result.image, width, height);
    }
    TransitionImageLayout (device, *result.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);


    return std::move (result);
}


static AllocatedImage CreateCopyImageOnCPU (const DeviceExtra& device, const ImageBase& image, uint32_t layerIndex = 0)
{
    const uint32_t width  = image.GetWidth ();
    const uint32_t height = image.GetHeight ();

    AllocatedImage dst (device, Image2D::Create (device, image.GetWidth (), image.GetHeight (), image.GetFormat (), VK_IMAGE_TILING_LINEAR, VK_IMAGE_USAGE_TRANSFER_DST_BIT, 1), DeviceMemory::CPU);
    TransitionImageLayout (device, *dst.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

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

        vkCmdCopyImage (
            single,
            image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            *dst.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1,
            &imageCopyRegion);
    }

    return dst;
}


std::vector<uint8_t> ReadImage (const std::filesystem::path& filePath, uint32_t components)
{
    int width, height, readComponents;

    unsigned char* imageData = stbi_load (filePath.u8string ().c_str (), &width, &height, &readComponents, components);

    if (ERROR (imageData == nullptr)) {
        throw std::runtime_error ("failed to load image");
    }

    std::vector<uint8_t> imageBytes (width * height * components);
    memcpy (imageBytes.data (), imageData, width * height * components);

    stbi_image_free (imageData);

    return imageBytes;
}

// copy image to cpu and compare with a reference
bool AreImagesEqual (const DeviceExtra& device, const ImageBase& image, const std::filesystem::path& expectedImage, uint32_t layerIndex)
{
    const uint32_t width      = image.GetWidth ();
    const uint32_t height     = image.GetHeight ();
    const uint32_t pixelCount = width * height;
    const uint32_t byteCount  = pixelCount * 4;

    AllocatedImage dst = CreateCopyImageOnCPU (device, image, layerIndex);


    std::vector<std::array<uint8_t, 4>> mapped (pixelCount);

    {
        MemoryMapping mapping (device, *dst.memory, 0, byteCount);
        memcpy (mapped.data (), mapping.Get (), byteCount);
    }

    std::vector<std::array<uint8_t, 4>> expected (pixelCount);
    int                                 expectedWidth, expectedHeight, expectedComponents;
    unsigned char*                      exepctedData = stbi_load (expectedImage.u8string ().c_str (), &expectedWidth, &expectedHeight, &expectedComponents, STBI_rgb_alpha);
    if (ERROR (expectedWidth != width || expectedHeight != height || expectedComponents != 4)) {
        stbi_image_free (exepctedData);
        return false;
    }

    memcpy (expected.data (), exepctedData, byteCount);
    stbi_image_free (exepctedData);

    return memcmp (expected.data (), mapped.data (), byteCount) == 0;
}


RawImageData::RawImageData (const DeviceExtra& device, const ImageBase& image, uint32_t layerIndex)
{
    width  = image.GetWidth ();
    height = image.GetHeight ();

    AllocatedImage dst = CreateCopyImageOnCPU (device, image, layerIndex);

    data.resize (width * height * components);

    {
        MemoryMapping mapping (device, *dst.memory, 0, width * height * components);
        memcpy (data.data (), mapping.Get (), width * height * components);
    }
}


RawImageData::RawImageData (const std::filesystem::path& path)
{
    int            w, h, comp;
    unsigned char* stbiData = stbi_load (path.u8string ().c_str (), &w, &h, &comp, STBI_rgb_alpha);

    width  = w;
    height = h;

    data.resize (width * height * components);

    memcpy (data.data (), stbiData, width * height * 4);
    stbi_image_free (stbiData);
}


bool RawImageData::operator== (const RawImageData& other) const
{
    if (width != other.width || height != other.height) {
        return false;
    }

    ASSERT (data.size () == other.data.size ());

    return memcmp (data.data (), other.data.data (), data.size ()) == 0;
}


uint32_t RawImageData::GetByteCount () const
{
    ASSERT (data.size () == width * height * components);
    return data.size ();
}


std::thread
SaveImageToFileAsync (const DeviceExtra& device, const ImageBase& image, const std::filesystem::path& filePath, uint32_t layerIndex)
{
    std::cout << "saving an image to" << filePath << std::endl;

    const uint32_t width  = image.GetWidth ();
    const uint32_t height = image.GetHeight ();

    AllocatedImage dst = CreateCopyImageOnCPU (device, image, layerIndex);

    constexpr uint32_t components = 4;

    std::vector<std::array<uint8_t, components>> mapped (width * height);

    {
        MemoryMapping mapping (device, *dst.memory, 0, width * height * components);
        memcpy (mapped.data (), mapping.Get (), width * height * components);
    }

    return std::thread ([=] () {
        stbi_write_png (filePath.u8string ().c_str (), width, height, components, mapped.data (), width * components);
        std::cout << "writing png done" << std::endl;
    });
};
