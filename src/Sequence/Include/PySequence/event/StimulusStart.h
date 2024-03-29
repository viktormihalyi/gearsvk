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

class SEQUENCE_API StimulusStart : public Base {
public:
    StimulusStart ()
        : Base (WM_USER, 0, 0)
    {
    }

    static uint32_t typeId;
};

} // namespace Event
} // namespace Gears