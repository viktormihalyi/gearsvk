#pragma once

#include "event/Base.h"

#pragma once

#include "event/Base.h"
#if defined(_WIN32)
#	include <windowsx.h>
#endif

namespace Gears {
	namespace Event {

		class StimulusEnd : public Base
		{
			StimulusEnd()
				:Base(WM_USER, 0, 0)
			{
			}
		public:
			GEARS_SHARED_CREATE_WITH_GETSHAREDPTR(StimulusEnd);

			static uint typeId;
		};

	}
}