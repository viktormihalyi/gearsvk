#ifndef ASSERT_HPP
#define ASSERT_HPP

#include <string>

#include "SourceLocation.hpp"

#ifndef NDEBUG
#define ASSERT(condition) (::Utils::detail::DebugAssertFunc (condition, "ASSERTION", #condition, {__FILE__, __LINE__, __func__}))
#define ERROR(condition) (::Utils::detail::DebugErrorFunc (condition, "ERROR", #condition, {__FILE__, __LINE__, __func__}))
#define BREAK(message) (::Utils::detail::DebugErrorFunc (true, "BREAK", message, {__FILE__, __LINE__, __func__}))
#else
#define ASSERT(condition) ((bool)condition)
#define ERROR(condition) ((bool)condition)
#define BREAK(message) (message)
#endif


#define ASSERT_THROW(cond)                               \
    if (ERROR (!(cond))) {                                \
        throw std::runtime_error ("precondition failed"); \
    }                                                     \
    (void)0

namespace Utils {

namespace detail {


bool DebugAssertFunc (bool condition, const char* message, const char* conditionString, const SourceLocation& location);

bool DebugErrorFunc (bool condition, const char* message, const char* conditionString, const SourceLocation& location);

void ShowAssertPopup (const std::string& title,
                      const std::string& message,
                      const std::string& sourceLocation,
                      bool&              wasIgnored);

} // namespace detail

} // namespace Utils


#endif