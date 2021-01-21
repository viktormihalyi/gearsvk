#ifndef RAWIMAGEDATA_HPP
#define RAWIMAGEDATA_HPP

#include <cstdlib>
#include <filesystem>
#include <optional>
#include <vector>

#include "DeviceExtra.hpp"
#include "Image.hpp"

class GEARSVK_API ImageData {
private:
    ImageData () = default;

public:
    size_t               components;
    size_t               width;
    size_t               height;
    std::vector<uint8_t> data;

    ImageData (const DeviceExtra& device, const Image& image, uint32_t layerIndex, std::optional<VkImageLayout> currentLayout = std::nullopt);
    ImageData (const DeviceExtra& device, const Image& image);
    ImageData (const std::filesystem::path& path, const uint32_t components = 4);

    static ImageData FromDataUint (const std::vector<uint8_t>& data, uint32_t width, uint32_t height, uint32_t components);
    static ImageData FromDataFloat (const std::vector<float>& data, uint32_t width, uint32_t height, uint32_t components);

    bool operator== (const ImageData& other) const;

    uint32_t GetByteCount () const;

    void ConvertBGRToRGB ();

    void SaveTo (const std::filesystem::path& path) const;
    void UploadTo (const DeviceExtra& device, const Image& image, std::optional<VkImageLayout> currentLayout = std::nullopt) const;
};

#endif