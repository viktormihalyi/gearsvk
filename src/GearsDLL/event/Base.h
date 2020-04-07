#pragma once

#include <windows.h>


namespace Gears {
namespace Event {

class Base {
protected:
    Base (uint message, uint wParam, uint lParam)
        : message (message), wParam (wParam), lParam (lParam)
    {
    }

public:
    GEARS_SHARED_CREATE_WITH_GETSHAREDPTR (Base);

public:
    uint message;
    uint wParam;
    uint lParam;
};

} // namespace Event
} // namespace Gears