#include "Assert.hpp"

#include "CommandLineFlag.hpp"
#include "CompilerDefinitions.hpp"
#include "MessageBox.hpp"

#include <iostream>
#include <memory>
#include <set>
#include <string>
#include <vector>


namespace Utils {

namespace detail {

static void DebugBreak ()
{
#if defined(COMPILER_MSVC)
    __debugbreak ();
#elif defined(COMPILER_CLANG)
    __builtin_debugtrap ();
#elif defined(COMPILER_GCC)
    __builtin_trap ();
#else
#error "unknown compiler for debug break"
#endif
}


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
            DebugBreak ();
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


static CommandLineOnOffFlag disableAssertsFlag (std::vector<std::string> { "--disableAsserts", "-a" }, "Disables asserts.");


bool DebugBreakAssertFunc (bool condition, const bool shouldBe, const char* message, const char* conditionString, const SourceLocation& location)
{
    if (condition != shouldBe) {
        const std::string assertLocation = location.ToString ();
        bool              ignored        = true;
        if (disableAssertsFlag.IsFlagOn ()) {
            std::cout << "[" << message << "] " << location.ToString () << std::endl;
        } else {
            ShowAssertPopup (message, conditionString, assertLocation, ignored);
        }
    }
    return condition;
}


bool LogAssertFunc (bool condition, const bool shouldBe, const char* message, const char* conditionString, const SourceLocation& location)
{
    if (condition != shouldBe) {
        //std::cout << "[" << message << "] " << location.ToString () << std::endl;
    }
    return condition;
}

} // namespace detail

} // namespace Utils