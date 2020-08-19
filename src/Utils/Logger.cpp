#include "Logger.hpp"

#include "Assert.hpp"
#include "Singleton.hpp"


#include <ctime>
#include <fstream>
#include <iostream>


namespace Log {

// change this to change the logger
#define LOGGERIMPL ConsoleLogger
#define LOGGERLEVEL Logger::LogLevel::Debug

class Logger {
public:
    enum LogLevel {
        Debug = 0,
        Info  = 1,
        Error = 2,
    };

public:
    virtual void ErrorImpl (const std::string&) = 0;
    virtual void InfoImpl (const std::string&)  = 0;
    virtual void DebugImpl (const std::string&) = 0;
};


static std::string GetCurrentTimeString ()
{
    time_t rawtime;
    time (&rawtime);
    struct tm* timeinfo = localtime (&rawtime);

    char buffer[80];
    strftime (buffer, sizeof (buffer), "%Y-%m-%d %H:%M:%S", timeinfo);

    return std::string (buffer);
}


class FileLogger final : public Logger {
    SINGLETON (FileLogger);

private:
    static void AppendToFile (const std::string& message)
    {
        std::ofstream ost;
        ost.open ("c:\\users\\viktor\\test.txt", std::ios::out | std::ios::app);

        if (GVK_ERROR (ost.fail ()))
            return;

        ost << message << std::endl;
        ost.close ();
    }

public:
    void ErrorImpl (const std::string& message) override
    {
        AppendToFile (std::string ("[GVK_ERROR ") + GetCurrentTimeString () + "] " + message);
    }

    void InfoImpl (const std::string& message) override
    {
        AppendToFile (std::string ("[INFO ") + GetCurrentTimeString () + "] " + message);
    }

    void DebugImpl (const std::string& message) override
    {
        AppendToFile (std::string ("[DEBUG ") + GetCurrentTimeString () + "] " + message);
    }
};


class ConsoleLogger final : public Logger {
    SINGLETON (ConsoleLogger);

public:
    void ErrorImpl (const std::string& message) override
    {
        std::cerr << "[GVK_ERROR " << GetCurrentTimeString () << "] " << message << std::endl;
    }

    void InfoImpl (const std::string& message) override
    {
        std::cout << "[INFO " << GetCurrentTimeString () << "] " << message << std::endl;
    }

    void DebugImpl (const std::string& message) override
    {
        std::cout << "[DEBUG " << GetCurrentTimeString () << "] " << message << std::endl;
    }
};


void Error (const std::string& message)
{
    if constexpr (LOGGERLEVEL <= Logger::LogLevel::Error) {
        LOGGERIMPL::Instance ().ErrorImpl (message);
    }
}

void Info (const std::string& message)
{
    if constexpr (LOGGERLEVEL <= Logger::LogLevel::Info) {
        LOGGERIMPL::Instance ().InfoImpl (message);
    }
}

void Debug (const std::string& message)
{
    if constexpr (LOGGERLEVEL <= Logger::LogLevel::Debug) {
        LOGGERIMPL::Instance ().DebugImpl (message);
    }
}

} // namespace Log
