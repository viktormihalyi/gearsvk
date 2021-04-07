#ifndef COMMANDLINEFLAG_HPP
#define COMMANDLINEFLAG_HPP

#include "GVKUtilsAPI.hpp"

#include <string>
#include <vector>

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
    static bool MatchOne (int argc, char** argv, Matcher& matcher, const std::string& flag);
};


class GVK_UTILS_API CommandLineOnOffFlag : Utils::CommandLineFlag {
private:
    bool                           on;
    const std::vector<std::string> flags;
    const std::string              helpText;

public:
    CommandLineOnOffFlag (const std::string& flag, const std::string& helpText = "");
    CommandLineOnOffFlag (const std::vector<std::string>& flags, const std::string& helpText = "");

    bool IsFlagOn () const;

    virtual std::string GetHelpText () override;

private:
    void Match (int argc, char** argv, Utils::Matcher& matcher);
};


} // namespace Utils


#endif