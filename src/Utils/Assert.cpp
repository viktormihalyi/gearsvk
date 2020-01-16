#include "Assert.hpp"

#include "CompilerDefinitions.hpp"
#include "MessageBox.hpp"

#include <set>
#include <string>

namespace Utils {

namespace detail {

static void ShowAssertPopup (const std::string& title,
                             const std::string& message,
                             const std::string& sourceLocation,
                             bool&              wasIgnored)
{
    static std::set<std::string> ignoredLocations;

    if (ignoredLocations.find (sourceLocation) != std::end (ignoredLocations)) {
        wasIgnored = true;
        return;
    }

    wasIgnored = false;

    bool ignoreNextTime = true;

    const MessageBox::Result result = MessageBox::Show (title, sourceLocation + "\n" + message);

    switch (result) {
        case MessageBox::Result::Yes:
            ignoreNextTime = false;

#if defined(COMPILER_MSVC)
            __debugbreak ();
#elif defined(COMPILER_GCC) || defined(COMPILER_CLANG)
            __builtin_debugtrap ();
#else
#error "unknown compiler for debug break"
#endif
            break;

        case MessageBox::Result::Third: // "Ignore"
            ignoreNextTime = true;
            break;

        case MessageBox::Result::No:
        case MessageBox::Result::Error:
            ignoreNextTime = false;
            break;
    }

    if (ignoreNextTime) {
        ignoredLocations.insert (sourceLocation);
    }
}


bool DebugAssertFunc (bool condition, const char* message, const char* conditionString, const SourceLocation& location)
{
    if (!condition) {
        const std::string assertLocation = location.ToString ();
        bool              ignored        = true;
        ShowAssertPopup (message, std::string (conditionString) + " is false", assertLocation, ignored);
    }
    return condition;
}


bool DebugErrorFunc (bool condition, const char* message, const char* conditionString, const SourceLocation& location)
{
    if (condition) {
        const std::string assertLocation = location.ToString ();
        bool              ignored        = true;
        ShowAssertPopup (message, std::string (conditionString) + " is true", assertLocation, ignored);
    }
    return condition;
}

} // namespace detail

} // namespace Utils