#ifndef UTILS_TIMER
#define UTILS_TIMER

#include "BuildType.hpp"

#include <chrono>
#include <iostream>
#include <string>

namespace Utils {

class TimerObserver {
public:
    using Duration = std::chrono::duration<double>;

    virtual void TimerEnded (Duration delta) = 0;
};


class TimerScope {
private:
    TimerObserver&                                              observer;
    std::chrono::time_point<std::chrono::high_resolution_clock> startTime;

public:
    TimerScope (TimerObserver& observer)
        : observer (observer)
        , startTime (std::chrono::high_resolution_clock::now ())
    {
    }

    ~TimerScope ()
    {
        observer.TimerEnded (std::chrono::high_resolution_clock::now () - startTime);
    }
};


class DummyTimerLogger final : public TimerObserver {
private:
    void TimerEnded (Duration) override
    {
    }
};


class DebugTimerLogger final : public TimerObserver {
private:
    std::string name;

public:
    DebugTimerLogger (const std::string& name = "")
        : name (name)
    {
    }

private:
    void TimerEnded (Duration delta) override
    {
        if constexpr (IsDebugBuild) {
            if (name.empty ()) {
                std::cout << "operation took " << delta.count () << " sec" << std::endl;
            } else {
                std::cout << name << " took " << delta.count () << " sec" << std::endl;
            }
        }
    }
};

} // namespace Utils

#endif