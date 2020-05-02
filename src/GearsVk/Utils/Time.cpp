#include "Time.hpp"

const std::chrono::time_point<TimePoint::Clock> TimePoint::ApplicationStartTime (TimePoint::Clock::now ());
