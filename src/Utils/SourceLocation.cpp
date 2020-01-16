#include "SourceLocation.hpp"

#include <sstream>

namespace Utils {

std::string SourceLocation::ToString () const
{
    std::stringstream ss;
    ss << file << ":" << line << " (" << function << ")";
    return ss.str ();
}

} // namespace Utils