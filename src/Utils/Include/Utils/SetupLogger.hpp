#ifndef UTILS_SETUPLOGGER_HPP
#define UTILS_SETUPLOGGER_HPP

#include "GVKUtilsAPI.hpp"

#include "spdlog/spdlog.h"

#include <string>

namespace Utils {

GVK_UTILS_API
std::shared_ptr<spdlog::logger> GetLogger ();

GVK_UTILS_API
std::shared_ptr<spdlog::logger> GetLogger (const std::string& customFileName);

} // namespace Utils


#endif
