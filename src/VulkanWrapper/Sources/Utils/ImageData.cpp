#include "ImageData.hpp"
#include "Commands.hpp"

#include "DeviceExtra.hpp"
#include "Utils/Assert.hpp"
#include "Utils/FileSystemUtils.hpp"

#pragma warning(push, 0)
#include "stb_image.h"
#include "stb_image_write.h"
#pragma warning(pop)

#include "VulkanUtils.hpp"

#include "spdlog/spdlog.h"

namespace GVK {

const ImageData ImageData::Empty { 4, 0, 0, {} };


ImageData::ImageData (size_t                      components,
                      size_t                      width,
                      size_t                      height,
                      const std::vector<uint8_t>& data)
    : components { components }
    , width { width }
    , height { height }
    , data { data }
{
}


ImageData::ImageData (const DeviceExtra& device, const Image& image, uint32_t layerIndex, std::optional<VkImageLayout> currentLayout)
    : components (4)
    , componentByteSize (GetEachCompontentSizeFromFormat (image.GetFormat ()))
    , width (image.GetWidth ())
    , height (image.GetHeight ())
{
    if (currentLayout)
        TransitionImageLayout (device, image, *currentLayout, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

    auto dataCopy = data;
    dataCopy.resize (width * height * components * componentByteSize);

    {
        Buffer dstBuffer (device.GetAllocator (), width * height * components * componentByteSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT, Buffer::MemoryLocation::CPU);
        
        {
            SingleTimeCommand single (device);
            image.CmdCopyLayerToBuffer (single, layerIndex, dstBuffer);
        }

        MemoryMapping mapping (device.GetAllocator (), dstBuffer);
        memcpy (dataCopy.data (), mapping.Get (), width * height * components * componentByteSize);
    }

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

        single.Record<CommandCopyImage> (
            image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            dst, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            std::vector<VkImageCopy> { imageCopyRegion });
    }

    if (currentLayout)
        TransitionImageLayout (device, image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, *currentLayout);

    data.resize (width * height * components * componentByteSize);

    {
        MemoryMapping mapping (device.GetAllocator (), dst);
        memcpy (data.data (), mapping.Get (), width * height * components * componentByteSize);
    }

    data = dataCopy;
}


ImageData::ImageData (const std::filesystem::path& path, const uint32_t components)
    : components (components)
    , componentByteSize (1)
{
    int            w, h, readComponents;
    unsigned char* stbiData = stbi_load (path.string ().c_str (), &w, &h, &readComponents, components);

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
    result.componentByteSize = 1;
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


ImageData::ComparisonResult ImageData::CompareTo (const ImageData& other) const
{
    if (GVK_ERROR (width != other.width || height != other.height)) {
        return ComparisonResult { false, nullptr };
    }

    if (*this == other) {
        return ComparisonResult { true, nullptr };
    }

    ComparisonResult result { true, std::make_unique<ImageData> (other) };
    std::fill (result.diffImage->data.begin (), result.diffImage->data.end (), 0);

    if (components == 4) {
        for (size_t i = 0; i < width * height * components; i += components) {
            if (memcmp (&data[i], &other.data[i], components) != 0) {
                const std::array<uint8_t, 4> pixel { data[i + 0], data[i + 1], data[i + 2], data[i + 3] };
                const std::array<uint8_t, 4> pixelOther { other.data[i + 0], other.data[i + 1], other.data[i + 2], other.data[i + 3] };

                const size_t diffSum = std::abs (pixel[0] - pixelOther[0]) + std::abs (pixel[1] - pixelOther[1]) + std::abs (pixel[2] - pixelOther[2]) + std::abs (pixel[3] - pixelOther[3]);

                if (diffSum > 1) {
                    const uint8_t diffR = std::abs (other.data[i + 0] - data[i + 0]);
                    const uint8_t diffG = std::abs (other.data[i + 1] - data[i + 1]);
                    const uint8_t diffB = std::abs (other.data[i + 2] - data[i + 2]);
                    const uint8_t diffA = std::abs (other.data[i + 3] - data[i + 3]);
                    const uint8_t maxDiff = std::max ({ diffR, diffG, diffB, diffA });
                    const double  maxDiffPerc = static_cast<double> (maxDiff) / 255;

                    result.diffImage->data[i + 0] = 255 * maxDiffPerc;
                    result.diffImage->data[i + 1] = 0;
                    result.diffImage->data[i + 2] = 0;
                    result.diffImage->data[i + 3] = 255;
                    result.equal                  = false;
                }
            }
        }
    } else if (components == 3) {
        for (size_t i = 0; i < width * height * components; i += components) {
            if (memcmp (&data[i], &other.data[i], components) != 0) {
                const std::array<uint8_t, 3> pixel { data[i + 0], data[i + 1], data[i + 2] };
                const std::array<uint8_t, 3> pixelOther { other.data[i + 0], other.data[i + 1], other.data[i + 2] };

                const size_t diffSum = std::abs (pixel[0] - pixelOther[0]) + std::abs (pixel[1] - pixelOther[1]) + std::abs (pixel[2] - pixelOther[2]);

                if (diffSum > 1) {
                    const uint8_t diffR       = std::abs (other.data[i + 0] - data[i + 0]);
                    const uint8_t diffG       = std::abs (other.data[i + 1] - data[i + 1]);
                    const uint8_t diffB       = std::abs (other.data[i + 2] - data[i + 2]);
                    const uint8_t maxDiff     = std::max ({ diffR, diffG, diffB });
                    const double  maxDiffPerc = static_cast<double> (maxDiff) / 255;

                    result.diffImage->data[i + 0] = 255 * maxDiffPerc;
                    result.diffImage->data[i + 1] = 0;
                    result.diffImage->data[i + 2] = 0;
                    result.equal                  = false;
                }
            }
        }
    } else {
        GVK_BREAK_STR ("not supported components");
    }
    
    return result;
}


uint32_t ImageData::GetByteCount () const
{
    GVK_ASSERT (data.size () == width * height * components);
    return data.size ();
}


void ImageData::SaveTo (const std::filesystem::path& path) const
{
    Utils::EnsureParentFolderExists (path);
    GVK_ASSERT (width * height * components * componentByteSize == data.size ());

    const int result = stbi_write_png (path.string ().c_str (), width, height, components, data.data (), width * components);

    if (GVK_VERIFY (result == 1)) {
        spdlog::info ("Saved image to {}.", path.string ());
    } else {
        spdlog::error ("Error saving image to {}.", path.string ());
    }
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

} // namespace GVK