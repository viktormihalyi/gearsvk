#pragma once

#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#endif

#ifndef WM_USER
#define WM_USER 0
#endif

#include "stdafx.h"

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