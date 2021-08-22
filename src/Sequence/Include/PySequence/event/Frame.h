#pragma once

#include "PySequence/event/Base.h"

#pragma once

#include "PySequence/event/Base.h"
#if defined(_WIN32)
#define NOMINMAX
#include <windowsx.h>
#endif

namespace Gears {
namespace Event {

class SEQUENCE_API Frame : public Base {
    Frame (uint32_t iFrame, float time)
#if _WIN32
        : Base (WM_USER, 0, 0), iFrame (iFrame), time (time)
#else
        : Base (0, 0, 0), iFrame (iFrame), time (time)
#endif
    {
    }

public:

    float       time;
    uint32_t        iFrame;
    static uint32_t typeId;
};

} // namespace Event
} // namespace Gears