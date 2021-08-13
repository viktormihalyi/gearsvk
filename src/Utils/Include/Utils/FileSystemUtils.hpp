#ifndef UTILS_FILESYSTEMUTILS_HPP
#define UTILS_FILESYSTEMUTILS_HPP

#include "GVKUtilsAPI.hpp"

#include <cstddef>
#include <filesystem>
#include <functional>
#include <optional>
#include <string>
#include <vector>


namespace Utils {

GVK_UTILS_API
std::optional<std::string> ReadTextFile (const std::filesystem::path& filePath);

GVK_UTILS_API
std::optional<std::vector<char>> ReadBinaryFile (const std::filesystem::path& filePath);

GVK_UTILS_API
bool WriteBinaryFile (const std::filesystem::path& filePath, const std::vector<uint8_t>& data);

GVK_UTILS_API
bool WriteBinaryFile (const std::filesystem::path& filePath, const void* data, size_t size);

GVK_UTILS_API
bool WriteTextFile (const std::filesystem::path& filePath, const std::string&);

GVK_UTILS_API
std::optional<std::vector<uint32_t>> ReadBinaryFile4Byte (const std::filesystem::path& filePath);

GVK_UTILS_API
void EnsureParentFolderExists (const std::filesystem::path& filePath);


} // namespace Utils

#endif