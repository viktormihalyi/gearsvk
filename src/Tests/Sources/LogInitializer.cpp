#include "Utils/StaticInit.hpp"
#include "Utils/SetupLogger.hpp"

StaticInit testsLogInitializer (std::bind (spdlog::set_default_logger, Utils::GetLogger ()));
