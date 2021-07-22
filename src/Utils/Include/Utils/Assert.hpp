#ifndef ASSERT_HPP
#define ASSERT_HPP

#include "GVKUtilsAPI.hpp"

#include <string>

#include "SourceLocation.hpp"

#define LOGASSERTS

#ifndef NDEBUG
#define GVK_ASSERT(condition) (::Utils::detail::DebugBreakAssertFunc (condition, true, "ASSERTION", #condition, { __FILE__, __LINE__, __func__ }))
#define GVK_VERIFY(condition) (::Utils::detail::DebugBreakAssertFunc (condition, true, "ASSERTION", #condition, { __FILE__, __LINE__, __func__ }))
#define GVK_ERROR(condition) (::Utils::detail::DebugBreakAssertFunc (condition, false, "GVK_ERROR", #condition, { __FILE__, __LINE__, __func__ }))
#define GVK_BREAK(message) (::Utils::detail::DebugBreakAssertFunc (true, false, "BREAK", message, { __FILE__, __LINE__, __func__ }))
#elif defined(LOGASSERTS)
#define GVK_ASSERT(condition) (::Utils::detail::LogAssertFunc (condition, true, "ASSERTION", #condition, { __FILE__, __LINE__, __func__ }))
#define GVK_VERIFY(condition) (::Utils::detail::LogAssertFunc (condition, true, "ASSERTION", #condition, { __FILE__, __LINE__, __func__ }))
#define GVK_ERROR(condition) (::Utils::detail::LogAssertFunc (condition, false, "GVK_ERROR", #condition, { __FILE__, __LINE__, __func__ }))
#define GVK_BREAK(message) (::Utils::detail::LogAssertFunc (true, false, "BREAK", message, { __FILE__, __LINE__, __func__ }))
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
bool DebugBreakAssertFunc (const bool condition, const bool shouldBe, const char* message, const char* conditionString, const SourceLocation& location);

GVK_UTILS_API
bool LogAssertFunc (const bool condition, const bool shouldBe, const char* message, const char* conditionString, const SourceLocation& location);

} // namespace detail

} // namespace Utils


#endif