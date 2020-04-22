#ifndef CALLEVERY_HPP
#define CALLEVERY_HPP

#include <atomic>
#include <cstdint>
#include <thread>
#include <utility>

class CallEvery {
private:
    std::atomic<bool>      stop;
    std::function<void ()> callback;
    std::thread            timerThread;
    const int32_t          waitNs;
    uint32_t               lastDiff;

    void ThreadFunc ()
    {
        callback ();

        // auto start = std::chrono::high_resolution_clock::now ();

        std::this_thread::sleep_for (std::chrono::nanoseconds (waitNs));

        // auto                                     end     = std::chrono::high_resolution_clock::now ();
        // std::chrono::duration<double, std::nano> elapsed = end - start;
        // const int32_t waitedNs   = elapsed.count ();
        // const int32_t waitedDiff = std::abs (waitedNs - waitNs);
        // std::cout << "Waited " << waitedNs << " ns instead of " << waitNs << " ns (diff is " << ((elapsed.count () - waitNs) * 1e6) << " ms)" << std::endl;
        // std::cout << "percent: " << static_cast<double> (waitNs + waitedDiff) / waitNs << " more" << std::endl;

        if (!stop) {
            ThreadFunc ();
        }
    }

public:
    CallEvery (double waitInSeconds, std::function<void ()> callback)
        : callback (callback)
        , waitNs (waitInSeconds * 1e9)
        , lastDiff (0)
        , timerThread (std::bind (&CallEvery::ThreadFunc, this))
        , stop (false)
    {
    }

    ~CallEvery ()
    {
        stop = true;
        timerThread.join ();
    }
};

#endif