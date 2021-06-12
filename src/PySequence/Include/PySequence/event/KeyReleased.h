#pragma once

#include "PySequence/event/Base.h"
#include <sstream>

namespace Gears {
	namespace Event {

		class PYSEQUENCE_API KeyReleased : public Base
		{
			KeyReleased(uint32_t message, uint32_t wParam, uint32_t lParam)
				:Base(message, wParam, lParam)
			{
				std::stringstream ss;
				ss << (char)wParam;
				_text = ss.str();
			}

		public:
			std::string _text;

			uint32_t key()
			{
				return wParam;
			}
			std::string text()
			{
				return _text;
			}
			static uint32_t typeId;
		};

	}
}