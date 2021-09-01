#ifndef SOURCELOCATION_HPP
#define SOURCELOCATION_HPP

#include "GVKUtilsAPI.hpp"

namespace Utils {

// TODO replace std::source_location when c++20 happens

struct GVK_UTILS_API SourceLocation {
    const char* file;
    int         line;
    const char* function;
};

} // namespace Utils

#endif