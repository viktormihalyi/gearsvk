#ifndef UTILS_IMAGELOADER_HPP
#define UTILS_IMAGELOADER_HPP

#include "Assert.hpp"

#include <filesystem>
#include <optional>


namespace Utils {

class Image {
public:
    using Data = unsigned char*;

    enum class Format {
        PNG,
        BMP,
        TGA,
        JPG
    };

public:
    const Data   data;
    const int    width;
    const int    height;
    const int    components;
    const Format format;

private:
    Image (Format format, Data data, int width, int height, int components);

public:
    ~Image ();

    static std::optional<Image> Load (const std::filesystem::path& imageFilePath);
    static void                 Write (const std::filesystem::path& outputFilePath, int width, int height, int components, const void* data);

private:
    static Format GetFormatFromExtension (const std::filesystem::path& filePath);
};


} // namespace Utils


#endif