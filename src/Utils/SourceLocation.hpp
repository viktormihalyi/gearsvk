#ifndef SOURCELOCATION_HPP
#define SOURCELOCATION_HPP

#include "UtilsDLLExport.hpp"
#include <string>

namespace Utils {

struct GEARSVK_UTILS_API SourceLocation {
    const char* file;
    int         line;
    const char* function;

    std::string ToString () const;
};

} // namespace Utils

#endif