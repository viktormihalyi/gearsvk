#if 0
#include "ImageLoader.hpp"
#include "Assert.hpp"

#include <string>

#define STBI_FAILURE_USERMSG
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"


namespace Utils {


Image::Image (Format format, Data data, int width, int height, int components)
    : data (data)
    , width (width)
    , height (height)
    , components (components)
    , format (format)
{
}


static std::optional<Image> Image::Load (const std::filesystem::path& imageFilePath)
{
    int  width, height, components;
    Data data = stbi_load (imageFilePath.u8string (), &width, &height, &components, 0);

    if (data == nullptr) {
        std::string_view failureReason = stbi_failure_reason ();
        (void)failureReason;
        return std::nullopt;
    }

    return Image (GetFormatFromExtension (imageFilePath), data, width, height, components);
}


/* static */ Image::Format Image::GetFormatFromExtension (const std::filesystem::path& filePath)
{
    const std::string extension = filePath.extension ();
    if (extension == ".png") {
        return Format::PNG;
    } else if (extension == ".bmp") {
        return Format::BMP;
    } else if (extension == ".tga") {
        return Format::TGA;
    } else if (extension == ".jpg") {
        return Format::JPG;
    } else {
        BREAK ("unexpected image extension");
        return Format::PNG;
    }
}

/* static */ void Image::Write (const std::filesystem::path& outputFilePath, int width, int height, int components, const void* data)
{
    ASSERT (width > 0);
    ASSERT (height > 0);
    ASSERT (components > 0);
    ASSERT (data != nullptr);

    switch (GetFormatFromExtension (outputFilePath)) {
        case Format::PNG:
            static const int stride_in_bytes = 0;
            ASSERT (stbi_write_png (outputFilePath.u8string (), width, height, components, data, stride_in_bytes) == 0);
            break;

        case Format::BMP:
            ASSERT (stbi_write_bmp (outputFilePath.u8string (), width, height, components, data) == 0);
            break;

        case Format::TGA:
            ASSERT (stbi_write_tga (outputFilePath.u8string (), width, height, components, data) == 0);
            break;

        case Format::JPG:
            static const int quality = 100;
            ASSERT (stbi_write_jpg (outputFilePath.u8string (), width, height, components, data, quality) == 0);
            break;

        default:
            BREAK ("unexpected image format type");
            break;
    }
}

Image::~Image ()
{
    stbi_image_free (data);
}


} // namespace Utils
#endif