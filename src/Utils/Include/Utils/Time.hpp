#ifndef TIMEPOINT_HPP
#define TIMEPOINT_HPP

#include "GVKUtilsAPI.hpp"

#include <chrono>

namespace GVK {

class TimePoint {
public:
    using Precision = std::chrono::nanoseconds;
    using Clock     = std::chrono::high_resolution_clock;

private:
    uint64_t nanoseconds;
#ifndef NDEBUG
    double seconds;
    double milliseconds;
    double microseconds;
#endif


public:
    GVK_UTILS_API TimePoint (uint64_t nanoseconds = 0)
        : nanoseconds (nanoseconds)
#ifndef NDEBUG
        , seconds (AsSeconds ())
        , milliseconds (AsMilliseconds ())
        , microseconds (AsMicroseconds ())
#endif
    {
    }

    double GVK_UTILS_API AsSeconds () const { return static_cast<double> (nanoseconds * 1.0e-9); }
    double GVK_UTILS_API AsMilliseconds () const { return static_cast<double> (nanoseconds * 1.0e-6); }
    double GVK_UTILS_API AsMicroseconds () const { return static_cast<double> (nanoseconds * 1.0e-3); }
    double GVK_UTILS_API AsNanoseconds () const { return static_cast<double> (nanoseconds); }

    GVK_UTILS_API operator uint64_t () const { return nanoseconds; }

    TimePoint GVK_UTILS_API operator- (TimePoint other) const
    {
        return TimePoint (nanoseconds - other.nanoseconds);
    }

    static TimePoint GVK_UTILS_API SinceEpoch ()
    {
        return std::chrono::duration_cast<Precision> (Clock::now ().time_since_epoch ()).count ();
    }

    static TimePoint GVK_UTILS_API SinceApplicationStart ()
    {
        return std::chrono::duration_cast<Precision> (Clock::now () - ApplicationStartTime).count ();
    }

    static const std::chrono::time_point<TimePoint::Clock> GVK_UTILS_API ApplicationStartTime;
};

using TimeDelta = TimePoint;

} // namespace GVK

#endif