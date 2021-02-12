#include "ImageData.hpp"

#include "DeviceExtra.hpp"

#pragma warning(push, 0)
#include "stb_image.h"
#include "stb_image_write.h"
#pragma warning(pop)

#include "VulkanUtils.hpp"

namespace GVK {

ImageData::ImageData (const DeviceExtra& device, const Image& image, uint32_t layerIndex, std::optional<VkImageLayout> currentLayout)
    : components (4)
{
    width  = image.GetWidth ();
    height = image.GetHeight ();

    if (currentLayout)
        TransitionImageLayout (device, image, *currentLayout, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

    Image2D dst (device.GetAllocator (), Image::MemoryLocation::CPU,
                 image.GetWidth (), image.GetHeight (),
                 image.GetFormat (),
                 //VK_FORMAT_R8G8B8A8_SRGB,
                 VK_IMAGE_TILING_LINEAR, VK_IMAGE_USAGE_TRANSFER_DST_BIT, 1);

    TransitionImageLayout (device, dst, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

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
            vkCmdCopyImage (commandBuffer,
                            image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                            dst, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                            1,
                            &imageCopyRegion);
        });
    }

    if (currentLayout)
        TransitionImageLayout (device, image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, *currentLayout);

    data.resize (width * height * components);

    {
        MemoryMapping mapping (device.GetAllocator (), dst);
        memcpy (data.data (), mapping.Get (), width * height * components);
    }
}


ImageData::ImageData (const std::filesystem::path& path, const uint32_t components)
    : components (components)
{
    int            w, h, readComponents;
    unsigned char* stbiData = stbi_load (path.u8string ().c_str (), &w, &h, &readComponents, components);

    if (GVK_ERROR (stbiData == nullptr)) {
        width  = 0;
        height = 0;
        return;
    }

    width  = w;
    height = h;

    // GVK_ASSERT (readComponents == components);

    data.resize (width * height * components);
    memcpy (data.data (), stbiData, width * height * components);

    stbi_image_free (stbiData);
}


ImageData ImageData::FromDataUint (const std::vector<uint8_t>& data, uint32_t width, uint32_t height, uint32_t components)
{
    GVK_ASSERT (data.size () == width * height * components);

    ImageData result;
    result.data       = data;
    result.width      = width;
    result.height     = height;
    result.components = components;
    return result;
}


static std::vector<uint8_t> ToUint (const std::vector<float>& data)
{
    std::vector<uint8_t> result;
    result.reserve (data.size ());
    for (float f : data) {
        uint8_t val;
        if (f < 0.f) {
            val = 0;
        } else if (f > 1.f) {
            val = -1;
        } else {
            val = f * 255.f;
        }
        result.push_back (val);
    }
    return result;
}


ImageData ImageData::FromDataFloat (const std::vector<float>& data, uint32_t width, uint32_t height, uint32_t components)
{
    return FromDataUint (ToUint (data), width, height, components);
}


bool ImageData::operator== (const ImageData& other) const
{
    if (width != other.width || height != other.height) {
        return false;
    }

    GVK_ASSERT (data.size () == other.data.size ());

    return memcmp (data.data (), other.data.data (), data.size ()) == 0;
}


uint32_t ImageData::GetByteCount () const
{
    GVK_ASSERT (data.size () == width * height * components);
    return data.size ();
}


void ImageData::SaveTo (const std::filesystem::path& path) const
{
    GVK_ASSERT (width * height * components == data.size ());

    const int result = stbi_write_png (path.u8string ().c_str (), width, height, components, data.data (), width * components);

    GVK_ASSERT (result == 1);
}


void ImageData::UploadTo (const DeviceExtra& device, const Image& image, std::optional<VkImageLayout> currentLayout) const
{
    if (currentLayout)
        TransitionImageLayout (device, image, *currentLayout, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    {
        Buffer stagingCPUMemory (device.GetAllocator (), width * height * components, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, Buffer::MemoryLocation::CPU);

        MemoryMapping bm (device.GetAllocator (), stagingCPUMemory);
        bm.Copy (data);

        CopyBufferToImage (device, stagingCPUMemory, image, width, height);
    }

    if (currentLayout)
        TransitionImageLayout (device, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, *currentLayout);
}


void ImageData::ConvertBGRToRGB ()
{
    for (size_t i = 0; i < width * height * components; i += components) {
        std::swap (data[i + 0], data[i + 2]);
    }
}

}