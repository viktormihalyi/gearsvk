#ifndef LOGGER_HPP
#define LOGGER_HPP


#include <string>

namespace Log {

void Error (const std::string& message);

void Info (const std::string& message);

void Debug (const std::string& message);

} // namespace Log

#endif