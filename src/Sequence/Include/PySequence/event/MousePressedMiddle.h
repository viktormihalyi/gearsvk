#pragma once

#include "PySequence/event/Base.h"
#if defined(_WIN32)
#define NOMINMAX
#include <windowsx.h>
#endif

namespace Gears {
namespace Event {

class SEQUENCE_API MousePressedMiddle : public Base {
    MousePressedMiddle (uint32_t message, uint32_t wParam, uint32_t lParam)
        : Base (message, wParam, lParam)
    {
#ifdef _WIN32
        x = GET_X_LPARAM (lParam);
        y = GET_Y_LPARAM (lParam);
#elif __linux__
        //TODO: Create right implemenation for linux
        x = lParam;
        y = lParam;
#endif
    }


    uint32_t x;
    uint32_t y;


public:
    uint32_t globalX () { return x; }
    uint32_t globalY () { return y; }

    static uint32_t typeId;
};

} // namespace Event
} // namespace Gears