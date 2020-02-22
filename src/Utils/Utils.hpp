#ifndef UTILS_HPP
#define UTILS_HPP

#include <cstddef>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>


namespace Utils {

extern const std::filesystem::path PROJECT_ROOT;

std::optional<std::string>       ReadTextFile (const std::filesystem::path& filePath);
std::optional<std::vector<char>> ReadBinaryFile (const std::filesystem::path& filePath);

} // namespace Utils

#endif