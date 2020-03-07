#ifndef LOGGER_HPP
#define LOGGER_HPP

#include "UtilsDLLExport.hpp"

#include <string>

namespace Log {

GEARSVK_UTILS_API
void Error (const std::string& message);

GEARSVK_UTILS_API
void Info (const std::string& message);

GEARSVK_UTILS_API
void Debug (const std::string& message);

} // namespace Log

#endif