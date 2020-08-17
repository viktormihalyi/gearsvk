#ifndef SOURCELOCATION_HPP
#define SOURCELOCATION_HPP

#include "GVKUtilsAPI.hpp"

#include <string>

namespace Utils {

// TODO replace std::source_location when c++20 happens

struct GVK_UTILS_API SourceLocation {
    const char* file;
    int         line;
    const char* function;

    std::string ToString () const;

    constexpr static SourceLocation Current (const char* file = __FILE__, int line = __LINE__)
    {
        return SourceLocation {file, line, "???"};
    }
};

} // namespace Utils

#endif