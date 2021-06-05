#pragma once

#include <algorithm>
#include <chrono>
#include <string>
#include <thread>

class SequenceRenderer;

class Ticker : public std::enable_shared_from_this<Ticker> {
    bool                                         live;
    std::chrono::high_resolution_clock::duration tickInterval;
    //std::chrono::time_point<std::chrono::high_resolution_clock, std::chrono::high_resolution_clock::duration> previousTickTimePoint;
    uint32_t         ticksPerFrame;
    int          ticksToGoInCurrentFrame;
    bool         vsync;
    std::thread* tickerThread;

    std::shared_ptr<SequenceRenderer> sequenceRenderer;
    Ticker (std::shared_ptr<SequenceRenderer> sequenceRenderer)
        : sequenceRenderer (sequenceRenderer), live (true) {}

public:
    GEARS_SHARED_CREATE (Ticker);

    void start (float tickInterval_s, float frameInterval_s);
    void stop ();
    void run ();

    void onBufferSwap ();
};
