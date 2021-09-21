#include "SetupLogger.hpp"
#include "StaticInit.hpp"

#include <stdio.h>
#include <time.h>

#include <optional>

#include "Utils/Assert.hpp"

#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/rotating_file_sink.h"


namespace Utils {

void SetupLogger ()
{
    static std::shared_ptr<spdlog::logger> logger;
    
    if (logger == nullptr) {
        time_t    now = time (0);
        struct tm tstruct;
        tstruct = *localtime (&now);

        const std::string logFileName = fmt::format ("GearsVk_{}-{}-{}_{}-{}-{}.txt", tstruct.tm_year, tstruct.tm_mon, tstruct.tm_mday, tstruct.tm_hour, tstruct.tm_min, tstruct.tm_sec);

        const size_t maxFileSize = 1024 * 1024 * 10;
        const size_t maxFiles    = 1000;

        std::vector<spdlog::sink_ptr> sinks;
        sinks.push_back (std::make_shared<spdlog::sinks::stdout_color_sink_st> ());
        sinks.push_back (std::make_shared<spdlog::sinks::rotating_file_sink_mt> (logFileName, maxFileSize, maxFiles));

        logger = std::make_shared<spdlog::logger> ("GearsVk", std::begin (sinks), std::end (sinks));
    }
    
    spdlog::set_default_logger (logger);

    spdlog::set_level (spdlog::level::trace);
    spdlog::set_pattern ("[%T] [%^%l%$] [t %t] %v");
}

} // namespace Utils


StaticInit utilsLogInitializer (Utils::SetupLogger);