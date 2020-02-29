#ifndef UTILS_HPP
#define UTILS_HPP

#include <cstddef>
#include <filesystem>
#include <functional>
#include <optional>
#include <set>
#include <string>
#include <vector>


namespace Utils {

extern const std::filesystem::path PROJECT_ROOT;

std::optional<std::string>       ReadTextFile (const std::filesystem::path& filePath);
std::optional<std::vector<char>> ReadBinaryFile (const std::filesystem::path& filePath);


template<typename SourceType, typename DestType>
std::set<DestType> ToSet (const std::vector<SourceType>& vec)
{
    std::set<DestType> result;
    std::transform (std::begin (vec), std::end (vec), std::inserter (result, std::end (result)), [] (const SourceType& x) {
        return DestType (x);
    });
    return result;
}


template<typename SourceType, typename DestType>
std::set<DestType> ToSet (const std::vector<SourceType>& vec, const std::function<DestType (const SourceType&)>& converter)
{
    std::set<DestType> result;
    std::transform (std::begin (vec), std::end (vec), std::inserter (result, std::end (result)), converter);
    return result;
}


template<typename T>
std::set<T> SetDiff (const std::set<T>& left, const std::set<T>& right)
{
    std::set<T> diff;
    std::set_difference (std::begin (left), std::end (left), std::begin (right), std::end (right), std::inserter (diff, std::end (diff)));
    return diff;
}

} // namespace Utils

#endif