#ifndef MESSAGEBOX_HPP
#define MESSAGEBOX_HPP

#include "GVKUtilsAPI.hpp"

#include <string>

enum class MessageBoxResult {
    Yes,
    No,
    Third,
    Error,
};

GVK_UTILS_API
MessageBoxResult ShowMessageBox (const std::string& title, const std::string& message);

#endif