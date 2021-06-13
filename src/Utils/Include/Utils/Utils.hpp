#ifndef UTILS_HPP
#define UTILS_HPP

#include "GVKUtilsAPI.hpp"

#include "Noncopyable.hpp"
#include <memory>

#include <algorithm>
#include <cstddef>
#include <filesystem>
#include <functional>
#include <iterator>
#include <mutex>
#include <optional>
#include <set>
#include <string>
#include <type_traits>
#include <vector>


#define PROJECT_ROOT ::Utils::GetProjectRoot ()


namespace Utils {

GVK_UTILS_API
std::filesystem::path GetProjectRoot ();

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
std::vector<std::string> SplitString (const std::string& str, const char delim, const bool keepEmpty = false);

GVK_UTILS_API
std::vector<std::string> SplitString (const std::string& str, const std::string& delim, const bool keepEmpty = false);

GVK_UTILS_API
std::string ReplaceAll (const std::string& str, const std::string& substringToReplace, const std::function<std::string ()>& replacementSubstring);

GVK_UTILS_API
bool StringContains (const std::string& str, const std::string& substr);

GVK_UTILS_API
void EnsureParentFolderExists (const std::filesystem::path& filePath);

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
        GVK_ASSERT (s != nullptr);
        result.push_back (static_cast<DestinationType> (*s));
    }
    return result;
}


template<typename SourceType, typename DestinationType>
std::vector<DestinationType> ConvertToHandles (const std::vector<std::shared_ptr<SourceType>>& src)
{
    std::vector<DestinationType> result;
    result.reserve (src.size ());
    for (const std::shared_ptr<SourceType>& s : src) {
        GVK_ASSERT (s != nullptr);
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


template<typename CastedType, typename Processor, typename Container>
void ForEach (Container&& container, const Processor& processor)
{
    for (auto& elem : container) {
        if (auto castedElem = dynamic_cast<CastedType> (elem)) {
            processor (castedElem);
        }
    }
}


template<typename CastedType, typename Processor, typename Container>
void ForEachP (Container& container, const Processor& processor)
{
    for (auto& elem : container) {
        if (auto castedElem = std::dynamic_pointer_cast<CastedType> (elem)) {
            processor (castedElem);
        }
    }
}

} // namespace Utils

#endif