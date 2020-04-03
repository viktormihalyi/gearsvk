#pragma once

#include "event/Base.h"
#include <sstream>

namespace Gears {
	namespace Event {

		class KeyPressed : public Base
		{
			KeyPressed(uint message, uint wParam, uint lParam)
				:Base(message, wParam, lParam)
			{
				std::stringstream ss;
				ss << (char)wParam;
				_text = ss.str();
			}
		public:
			GEARS_SHARED_CREATE_WITH_GETSHAREDPTR(KeyPressed);

		public:
			std::string _text;

			uint key()
			{
				return wParam;
			}
			std::string text()
			{
 				return _text;
			}
			static uint typeId;
		};

	}
}