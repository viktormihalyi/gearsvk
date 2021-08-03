#ifndef COMMANDLINEFLAG_HPP
#define COMMANDLINEFLAG_HPP

#include "GVKUtilsAPI.hpp"

#include <string>
#include <vector>
#include <optional>
#include <sstream>
#include <tuple>

namespace Utils {


class GVK_UTILS_API Matcher {
public:
    virtual ~Matcher () = default;

    virtual void SignMatch (int index) = 0;
};

class GVK_UTILS_API CommandLineFlag {
public:
    CommandLineFlag ();
    virtual ~CommandLineFlag () = default;


    virtual void Match (int argc, char** argv, Matcher& matcher) = 0;

    virtual std::string GetHelpText () { return ""; }

    static void MatchAll (int argc, char** argv, bool printUnusedWarning = true);

protected:
    static std::optional<size_t> MatchOne (int argc, char** argv, Matcher& matcher, const std::string& flag);
};


class CommandLineOnOffFlag : public CommandLineFlag {
protected:
    bool                           on;
    const std::vector<std::string> flags;
    const std::string              helpText;

public:
    GVK_UTILS_API CommandLineOnOffFlag (const std::string& flag, const std::string& helpText = "");
    GVK_UTILS_API CommandLineOnOffFlag (const std::vector<std::string>& flags, const std::string& helpText = "");

    virtual GVK_UTILS_API ~CommandLineOnOffFlag () override = default;

    bool GVK_UTILS_API IsFlagOn () const;

    virtual std::string GVK_UTILS_API GetHelpText () override;

private:
    virtual void Match (int argc, char** argv, Utils::Matcher& matcher) override;
    virtual void MatchParameters (int argc, char** argv, Utils::Matcher& matcher, size_t matched) {}
};


/*
inline std::istringstream& get_istringstream ()
{
    static thread_local std::istringstream stream;
    stream.str ("");
    return stream;
}


template<typename T>
inline T from_string (const std::string& s)
{
    auto& iss (get_istringstream ());
    iss.str (s);
    T result;
    iss >> result;
    return result;
}

template<typename... Parameters>
class GVK_UTILS_API CommandLineFlagWithNumberParameter : public CommandLineOnOffFlag {
private:
    std::tuple<Parameters...> values;
    int64_t number;

public:
    CommandLineFlagWithNumberParameter (const std::string& flag, const std::string& helpText = "")
        : CommandLineOnOffFlag { std::vector<std::string> { flag }, helpText }
        , number { 0 }
    {
    }

    int64_t GetValue () { return number; }

private:
    // virtual void MatchParameters (int argc, char** argv, Utils::Matcher& matcher, size_t matched) override;

    virtual void MatchParameters (int argc, char** argv, Utils::Matcher& matcher, size_t matched) override
    {

        if (GVK_VERIFY (matched + 1 < argc)) {
            for (int i = 0; i < std::tuple_size<decltype (values)> {}; ++i) {
                std::get<i> (values) = from_string (argv[matched+i]);
            }
            number = std::stoi (argv[matched + 1]);
            matcher.SignMatch (matched + 1);
        }
    }
};
*/


} // namespace Utils


#endif