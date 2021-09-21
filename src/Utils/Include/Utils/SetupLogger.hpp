#ifndef UTILS_SETUPLOGGER_HPP
#define UTILS_SETUPLOGGER_HPP

#include "GVKUtilsAPI.hpp"

#include "spdlog/spdlog.h"

namespace Utils {

GVK_UTILS_API
std::shared_ptr<spdlog::logger> GetLogger ();

} // namespace Utils


#endif
