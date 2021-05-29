#pragma once

#include "event/Base.h"

namespace Gears {
namespace Event {

class MousePressedLeft : public Base {
    MousePressedLeft (uint message, uint wParam, uint lParam)
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

public:
    GEARS_SHARED_CREATE_WITH_GETSHAREDPTR (MousePressedLeft);

    uint x;
    uint y;

public:
    uint globalX () { return x; }
    uint globalY () { return y; }

    static uint typeId;
};

} // namespace Event
} // namespace Gears