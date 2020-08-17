#ifndef UUID_HPP
#define UUID_HPP

#include <string>

#include "GVKUtilsAPI.hpp"

namespace GearsVk {

class GVK_UTILS_API UUID {
private:
    std::string value;

public:
    UUID ();

    operator std::string () const { return value; }

    const std::string& GetValue () const { return value; }

    bool operator== (const UUID& other) const { return value == other.value; }
    bool operator!= (const UUID& other) const { return value != other.value; }
};

} // namespace GearsVk


namespace std {
template<>
struct hash<GearsVk::UUID> {
    std::size_t operator() (const GearsVk::UUID& uuid) const noexcept
    {
        return std::hash<std::string> () (uuid.GetValue ());
    }
};
} // namespace std

#endif