#ifndef UTILS_TIMER
#define UTILS_TIMER

#include "UtilsDLLExport.hpp"

#include <chrono>
#include <iostream>
#include <string>

namespace Utils {

class GEARSVK_UTILS_API TimerObserver {
public:
    using Duration = std::chrono::duration<double>;

    virtual void TimerEnded (Duration delta) = 0;
};


class GEARSVK_UTILS_API TimerScope {
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


class GEARSVK_UTILS_API DummyTimerLogger final : public TimerObserver {
private:
    void TimerEnded (Duration) override
    {
    }
};


class GEARSVK_UTILS_API TimerLogger final : public TimerObserver {
private:
    std::string name;

public:
    TimerLogger (const std::string& name = "")
        : name (name)
    {
    }

private:
    void TimerEnded (Duration delta) override
    {
        if (name.empty ()) {
            std::cout << "operation took " << delta.count () << " sec" << std::endl;
        } else {
            std::cout << name << " took " << delta.count () << " sec" << std::endl;
        }
    }
};

} // namespace Utils

#endif