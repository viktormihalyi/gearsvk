#ifndef SOURCELOCATION_HPP
#define SOURCELOCATION_HPP

#include <string>

namespace Utils {

struct SourceLocation {
    const char* file;
    int         line;
    const char* function;

    std::string ToString () const;
};

} // namespace Utils

#endif