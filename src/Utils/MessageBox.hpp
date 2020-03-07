#ifndef MESSAGEBOX_HPP
#define MESSAGEBOX_HPP

#include "UtilsDLLExport.hpp"

#include <string>

// cross platform messagebox with 3 buttons using SDL

namespace MessageBox {

enum class Result {
    Yes,
    No,
    Third,
    Error,
};

GEARSVK_UTILS_API
Result Show (const std::string& title, const std::string& message);

} // namespace MessageBox

#endif