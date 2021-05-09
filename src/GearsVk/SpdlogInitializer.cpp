#include "StaticInit.hpp"

#include "spdlog/spdlog.h"

StaticInit spdlogInitializer  ([] () {
    spdlog::set_level (spdlog::level::debug);
    spdlog::set_pattern ("[%H:%M:%S %z] [%n] [%^---%L---%$] [thread %t] %v");
});