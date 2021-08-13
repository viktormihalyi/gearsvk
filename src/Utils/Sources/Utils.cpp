#include "Utils.hpp"

#include <cstring>
#include <string>


namespace Utils {


std::vector<std::string> SplitString (const std::string& str, const char delim, const bool keepEmpty)
{
    std::vector<std::string> result;

    size_t prev = 0;
    size_t pos  = 0;

    do {
        pos = str.find (delim, prev);
        if (pos == std::string::npos) {
            pos = str.length ();
        }

        std::string token = str.substr (prev, pos - prev);

        if (keepEmpty || !token.empty ()) {
            result.push_back (std::move (token));
        }

        prev = pos + 1;

    } while (pos < str.length () && prev < str.length ());

    return result;
}


std::vector<std::string> SplitString (const std::string& str, const std::string& delim, const bool keepEmpty)
{
    std::vector<std::string> result;

    size_t prev = 0;
    size_t pos  = 0;

    do {
        pos = str.find (delim, prev);
        if (pos == std::string::npos) {
            pos = str.length ();
        }

        std::string token = str.substr (prev, pos - prev);

        if (keepEmpty || !token.empty ()) {
            result.push_back (std::move (token));
        }

        prev = pos + delim.length ();

    } while (pos < str.length () && prev < str.length ());

    return result;
}


std::string ReplaceAll (const std::string& str, const std::string& substringToReplace, const std::function<std::string ()>& replacementSubstring)
{
    const std::vector<std::string> split = SplitString (str, substringToReplace, true);

    std::string result;

    for (const std::string& s : split) {
        if (s == substringToReplace) {
            result += replacementSubstring ();
        } else {
            result += s;
        }
    }

    return result;
}


bool StringContains (const std::string& str, const std::string& substr)
{
    return str.find (substr) != std::string::npos;
}


} // namespace Utils