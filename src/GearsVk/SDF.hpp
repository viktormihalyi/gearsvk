#ifndef SDF_HPP
#define SDF_HPP

#include <cstdint>
#include <filesystem>
#include <vector>

#include "GearsVkAPI.hpp"

GEARSVK_API
std::vector<float> GetGlyphSDF32x32x1 (const std::filesystem::path& fontFile, uint32_t unicode);

GEARSVK_API
std::vector<float> GetGlyphMDF16x16x3 (const std::filesystem::path& fontFile, uint32_t unicode);


#endif
