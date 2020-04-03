#pragma once

#include "event/Base.h"
#if defined(_WIN32)
#	include <windowsx.h>
#endif

namespace Gears {
	namespace Event {

		class MouseReleasedLeft : public Base
		{
			MouseReleasedLeft(uint message, uint wParam, uint lParam, uint screenw, uint screenh)
				:Base(message, wParam, lParam)
			{
#ifdef _WIN32
				x = GET_X_LPARAM(lParam);
				y = GET_Y_LPARAM(lParam);
#elif __linux__
//TODO: Create right implemenation for linux
				x = lParam;
				y = lParam;
#endif
				xPercent = (float)x / screenw;
				yPercent = (float)y / screenh;
			}
		public:
			GEARS_SHARED_CREATE_WITH_GETSHAREDPTR(MouseReleasedLeft);

			uint x;
			uint y;
			float xPercent;
			float yPercent;

		public:
			uint globalX(){ return x; }
			uint globalY(){ return y; }

			float globalPercentX(){ return xPercent; }
			float globalPercentY(){ return yPercent; }

			static uint typeId;
		};

	}
}