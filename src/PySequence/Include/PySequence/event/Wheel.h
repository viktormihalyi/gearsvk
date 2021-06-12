#pragma once

#include "PySequence/event/Base.h"

#pragma once

#include "PySequence/event/Base.h"

#if defined(_WIN32)
#define NOMINMAX
#include <windows.h>
#endif

namespace Gears {
namespace Event {

class PYSEQUENCE_API Wheel : public Base {
    Wheel (uint32_t message, uint32_t wParam, uint32_t lParam)
        : Base (message, wParam, lParam)
    {
#ifdef _WIN32
        x  = GET_X_LPARAM (lParam);
        y  = GET_Y_LPARAM (lParam);
        dX = GET_X_LPARAM (wParam);
        dY = GET_Y_LPARAM (wParam);
#elif __linux__
        //TODO: Create right implemenation for linux
        x  = lParam;
        y  = lParam;
        dX = wParam;
        dY = wParam;
#endif
    }

    uint32_t x;
    uint32_t y;
    int  dX;
    int  dY;


public:
    uint32_t globalX () { return x; }
    uint32_t globalY () { return y; }
    int  deltaX () { return dX; }
    int  deltaY () { return dY; }


    static uint32_t typeId;
};

} // namespace Event
} // namespace Gears