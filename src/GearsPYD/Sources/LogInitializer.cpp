#include "RenderGraph/Utils/StaticInit.hpp"
#include "RenderGraph/Utils/SetupLogger.hpp"

StaticInit gearsPydLogInitializer (std::bind (spdlog::set_default_logger, Utils::GetLogger ()));
