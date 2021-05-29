#ifndef MESSAGEBOX_HPP
#define MESSAGEBOX_HPP

#include "GVKUtilsAPI.hpp"

#include <string>

namespace MessageBox {

enum class Result {
    Yes,
    No,
    Third,
    Error,
};

GVK_UTILS_API
Result Show (const std::string& title, const std::string& message);

} // namespace MessageBox

#endif