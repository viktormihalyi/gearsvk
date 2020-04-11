#ifndef TIME_HPP
#define TIME_HPP

#include <chrono>

class TimePoint {
public:
    using Precision = std::chrono::nanoseconds;
    using Clock     = std::chrono::high_resolution_clock;

    const uint64_t nanoseconds;

    TimePoint (uint64_t nanoseconds)
        : nanoseconds (nanoseconds)
    {
    }

    double AsSeconds () const { return nanoseconds * 1.0e-9; }
    double AsMilliseconds () const { return nanoseconds * 1.0e-6; }
    double AsMicroseconds () const { return nanoseconds * 1.0e-3; }
    double AsNanoseconds () const { return nanoseconds; }

    operator uint64_t () const { return nanoseconds; }

    static TimePoint SinceEpoch ()
    {
        return std::chrono::duration_cast<Precision> (Clock::now ().time_since_epoch ()).count ();
    }

    static TimePoint SinceApplicationStart ()
    {
        return std::chrono::duration_cast<Precision> (Clock::now () - ApplicationStartTime).count ();
    }

private:
    static const std::chrono::time_point<Clock> ApplicationStartTime;
};

const std::chrono::time_point<TimePoint::Clock> TimePoint::ApplicationStartTime (TimePoint::Clock::now ());


#endif