#include "Utils/StaticInit.hpp"
#include "Utils/SetupLogger.hpp"

StaticInit gearsPydLogInitializer (std::bind (spdlog::set_default_logger, Utils::GetLogger ()));
