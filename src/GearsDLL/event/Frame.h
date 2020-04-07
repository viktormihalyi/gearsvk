#pragma once

#include "event/Base.h"

#pragma once

#include "event/Base.h"
#if defined(_WIN32)
#	include <windowsx.h>
#endif

namespace Gears {
	namespace Event {

		class Frame : public Base
		{
			Frame(uint iFrame, float time)
				:Base(WM_USER, 0, 0), iFrame(iFrame), time(time)
			{
			}
		public:
			GEARS_SHARED_CREATE_WITH_GETSHAREDPTR(Frame);

			float time;
			uint iFrame;
			static uint typeId;
		};

	}
}