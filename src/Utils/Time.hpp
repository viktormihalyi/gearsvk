#ifndef TIMEPOINT_HPP
#define TIMEPOINT_HPP

#include "GVKUtilsAPI.hpp"

#include <chrono>

namespace GVK {

class GVK_UTILS_API TimePoint {
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
    TimePoint (uint64_t nanoseconds = 0)
        : nanoseconds (nanoseconds)
#ifndef NDEBUG
        , seconds (AsSeconds ())
        , milliseconds (AsMilliseconds ())
        , microseconds (AsMicroseconds ())
#endif
    {
    }

    double AsSeconds () const { return static_cast<double> (nanoseconds * 1.0e-9); }
    double AsMilliseconds () const { return static_cast<double> (nanoseconds * 1.0e-6); }
    double AsMicroseconds () const { return static_cast<double> (nanoseconds * 1.0e-3); }
    double AsNanoseconds () const { return static_cast<double> (nanoseconds); }

    operator uint64_t () const { return nanoseconds; }

    TimePoint operator- (TimePoint other) const
    {
        return TimePoint (nanoseconds - other.nanoseconds);
    }

    static TimePoint SinceEpoch ()
    {
        return std::chrono::duration_cast<Precision> (Clock::now ().time_since_epoch ()).count ();
    }

    static TimePoint SinceApplicationStart ()
    {
        return std::chrono::duration_cast<Precision> (Clock::now () - ApplicationStartTime).count ();
    }

    static const std::chrono::time_point<TimePoint::Clock> ApplicationStartTime;
};

using TimeDelta = TimePoint;

} // namespace GVK

#endif