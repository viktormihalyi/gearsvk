#ifndef TIME_HPP
#define TIME_HPP

#include <chrono>

class Time {
public:
    using Precision = std::chrono::nanoseconds;
    using Clock     = std::chrono::high_resolution_clock;

    struct Point {
        const uint64_t nanoseconds;

        Point (uint64_t nanoseconds)
            : nanoseconds (nanoseconds)
        {
        }

        double AsSeconds () const { return nanoseconds * 1.0e-9; }
        double AsMilliseconds () const { return nanoseconds * 1.0e-6; }
        double AsMicroseconds () const { return nanoseconds * 1.0e-3; }
        double AsNanoseconds () const { return nanoseconds; }

        operator uint64_t () const { return nanoseconds; }
    };

    static Point SinceEpoch ()
    {
        return std::chrono::duration_cast<Precision> (Clock::now ().time_since_epoch ()).count ();
    }

    static Point SinceApplicationStart ()
    {
        return std::chrono::duration_cast<Precision> (Clock::now () - ApplicationStartTime).count ();
    }

private:
    static const std::chrono::time_point<Clock> ApplicationStartTime;
};

const std::chrono::time_point<Time::Clock> Time::ApplicationStartTime (Time::Clock::now ());


#endif