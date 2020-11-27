#ifndef ASSERT_HPP
#define ASSERT_HPP

#include "GVKUtilsAPI.hpp"

#include <string>

#include "SourceLocation.hpp"

#if !defined(NDEBUG) || defined(FORCEDEBUGMODE)
#define GVK_ASSERT(condition) (::Utils::detail::DebugAssertFunc (condition, true, "ASSERTION", #condition, { __FILE__, __LINE__, __func__ }))
#define GVK_VERIFY(condition) (::Utils::detail::DebugAssertFunc (condition, true, "ASSERTION", #condition, { __FILE__, __LINE__, __func__ }))
#define GVK_ERROR(condition) (::Utils::detail::DebugAssertFunc (condition, false, "GVK_ERROR", #condition, { __FILE__, __LINE__, __func__ }))
#define GVK_BREAK(message) (::Utils::detail::DebugAssertFunc (true, false, "BREAK", message, { __FILE__, __LINE__, __func__ }))
#else
#define GVK_ASSERT(condition)
#define GVK_VERIFY(condition) ((bool)(condition))
#define GVK_ERROR(condition) ((bool)(condition))
#define GVK_BREAK(message)
#endif


#define GVK_ASSERT_THROW(cond)                            \
    if (GVK_ERROR (!(cond))) {                            \
        throw std::runtime_error ("precondition failed"); \
    }                                                     \
    (void)0

namespace Utils {

namespace detail {

GVK_UTILS_API
bool DebugAssertFunc (const bool condition, const bool shouldBe, const char* message, const char* conditionString, const SourceLocation& location);

} // namespace detail

} // namespace Utils


#endif