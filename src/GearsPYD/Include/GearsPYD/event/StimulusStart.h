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

class StimulusStart : public Base {
public:
    StimulusStart ()
        : Base (WM_USER, 0, 0)
    {
    }

    static uint32_t typeId;
};

} // namespace Event
} // namespace Gears