#pragma once

#include "event/Base.h"

#pragma once

#include "event/Base.h"
#if defined(_WIN32)
#define NOMINMAX
#include <windowsx.h>
#endif

namespace Gears {
namespace Event {

class StimulusEnd : public Base {
    StimulusEnd ()
        : Base (WM_USER, 0, 0)
    {
    }

public:
    GEARS_SHARED_CREATE (StimulusEnd);

    static uint typeId;
};

} // namespace Event
} // namespace Gears