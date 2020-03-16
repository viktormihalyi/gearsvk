#ifndef UTILS_HPP
#define UTILS_HPP

#include <algorithm>
#include <cstddef>
#include <filesystem>
#include <functional>
#include <iterator>
#include <optional>
#include <set>
#include <string>
#include <vector>


#define PROJECT_ROOT ::Utils::GetProjectRoot ()


namespace Utils {

std::filesystem::path GetProjectRoot ();

std::optional<std::string> ReadTextFile (const std::filesystem::path& filePath);

std::optional<std::vector<char>> ReadBinaryFile (const std::filesystem::path& filePath);

std::optional<std::vector<uint32_t>> ReadBinaryFile4Byte (const std::filesystem::path& filePath);


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


template<typename SourceType, typename DestinationType>
std::vector<DestinationType> ConvertToHandles (const std::vector<std::unique_ptr<SourceType>>& src)
{
    std::vector<DestinationType> result;
    result.reserve (src.size ());
    for (const std::unique_ptr<SourceType>& s : src) {
        ASSERT (s != nullptr);
        result.push_back (static_cast<DestinationType> (*s));
    }
    return result;
}


template<typename SourceType, typename DestinationType>
std::vector<DestinationType> ConvertToHandles (const std::vector<std::reference_wrapper<SourceType>>& src)
{
    std::vector<DestinationType> result;
    result.reserve (src.size ());
    for (const std::reference_wrapper<SourceType>& s : src) {
        result.push_back (static_cast<DestinationType> (s.get ()));
    }
    return result;
}


} // namespace Utils

#endif