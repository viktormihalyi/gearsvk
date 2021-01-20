#ifndef UTILS_HPP
#define UTILS_HPP

#include "GVKUtilsAPI.hpp"

#include "Noncopyable.hpp"
#include "Ptr.hpp"

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


class NoValueT {
};

extern NoValueT NoValue;

template<typename T>
class Opt {
private:
    bool hasValue;
    T    value;

public:
    Opt ()
        : hasValue (false)
        , value ()
    {
    }

    Opt (NoValueT)
        : hasValue (false)
        , value ()
    {
    }

    Opt (T&& value)
        : hasValue (true)
        , value (std::move (value))
    {
    }

    Opt (Opt&& other)
        : hasValue (other.hasValue)
        , value (std::move (other.value))
    {
        other.hasValue = false;
    }

    Opt (const Opt& other)
        : hasValue (other.hasValue)
        , value (other.value)
    {
    }

    Opt& operator= (const Opt& other)
    {
        if (this != &other) {
            Clear ();
            hasValue = other.hasValue;
            value    = other.value;
        }
        return *this;
    }

    Opt& operator= (Opt&& other)
    {
        if (this != &other) {
            Clear ();
            hasValue = other.hasValue;
            value    = std::move (other.value);

            other.hasValue = false;
        }
        return *this;
    }

    Opt& operator= (T&& otherValue)
    {
        if (this != &other) {
            Clear ();
            hasValue = true;
            value    = std::move (otherValue);
        }
        return *this;
    }

    [[nodiscard]] bool operator== (const Opt& other) const
    {
        if (!hasValue && hasValue == other.hasValue) {
            return true;
        }

        return value == other.value;
    }

    [[nodiscard]] bool operator!= (const Opt& other) const { return !(*this == other); }

    [[nodiscard]] T&       operator* () { return Get (); }
    [[nodiscard]] const T& operator* () const { return Get (); }

    [[nodiscard]] T&       operator() () { return Get (); }
    [[nodiscard]] const T& operator() () const { return Get (); }

    [[nodiscard]] T&& Pass () const
    {
        if (!hasValue)
            hasValue = false;
        return std::move (Get ());
    }

    [[nodiscard]] T& Get ()
    {
        if (!hasValue) {
            throw std::runtime_error ("empty optional");
        }
        return value;
    }

    [[nodiscard]] const T& Get () const
    {
        if (!hasValue) {
            throw std::runtime_error ("empty optional");
        }
        return value;
    }

    template<typename U>
    [[nodiscard]] T& GetOr (U&& defaultValue)
    {
        if (!hasValue) {
            return defaultValue;
        }
        return value;
    }


    [[nodiscard]] bool HasValue () const { return hasValue; }
    [[nodiscard]] bool IsEmpty () const { return !hasValue; }

    void Clear ()
    {
        if (hasValue) {
            hasValue = false;
            value.~T ();
        }
    }

    template<class... Parameters>
    void Create (Parameters&&... parameters)
    {
        Clear ();
        value = T (std::forward<Parameters> (parameters)...);
    }

    [[nodiscard]] std::optional<T> MoveToStd ()
    {
        if (!hasValue) {
            return std::nullopt;
        }
        return std::optional<T> (std::move (value));
    }

    [[nodiscard]] operator std::optional<T> ()
    {
        return MoveToStd ();
    }
};


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
void ForEach (Container& container, const Processor& processor)
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