#ifndef MESSAGEBOX_HPP
#define MESSAGEBOX_HPP

#include <string>

namespace MessageBox {

enum class Result {
    Yes,
    No,
    Third,
    Error,
};

Result Show (const std::string& title, const std::string& message);

} // namespace MessageBox

#endif