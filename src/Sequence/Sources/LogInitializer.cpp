#include "Utils/StaticInit.hpp"
#include "Utils/SetupLogger.hpp"

StaticInit sequenceLogInitializer (std::bind (spdlog::set_default_logger, Utils::GetLogger ()));
