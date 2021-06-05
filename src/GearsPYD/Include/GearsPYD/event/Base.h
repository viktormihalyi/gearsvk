#pragma once

#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#endif

#ifndef WM_USER
#define WM_USER 0
#endif

#include <cstdint>

namespace Gears {
namespace Event {

class Base {
protected:
    Base (uint32_t message, uint32_t wParam, uint32_t lParam)
        : message (message), wParam (wParam), lParam (lParam)
    {
    }

public:
    uint32_t message;
    uint32_t wParam;
    uint32_t lParam;
};

} // namespace Event
} // namespace Gears