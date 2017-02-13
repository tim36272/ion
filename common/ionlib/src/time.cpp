/*
This file is part of Ionlib.  Copyright (C) 2016  Tim Sweet

Ionlib is free software : you can redistribute it and / or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Ionlib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Ionlib.If not, see <http://www.gnu.org/licenses/>.
*/
#include "ionlib/time.h"
#include <thread>
#include <chrono>
namespace ion
{
	static std::chrono::high_resolution_clock::time_point begin_g;
	static bool time_initialized_g = false;
	void TimeInit()
	{
		begin_g = std::chrono::high_resolution_clock::now();
	}
	//fast-query timer, may be the number of seconds since the program started
	double TimeGet()
	{
		if (!time_initialized_g)
		{
			TimeInit();
		}
		std::chrono::high_resolution_clock::time_point now = std::chrono::high_resolution_clock::now();
		return std::chrono::duration_cast<std::chrono::duration<double>>(now - begin_g).count();
	}
	//gives time since January 1, 1970 at midnight
	double TimeGetEpoch()
	{
		std::chrono::high_resolution_clock::time_point now = std::chrono::high_resolution_clock::now();
		return std::chrono::duration_cast<std::chrono::duration<double>>(now.time_since_epoch()).count();
	}
	void ThreadSleep(uint32_t milliseconds)
	{
		std::chrono::duration<double, std::milli> duration(milliseconds*1000.0);
		std::this_thread::sleep_for(duration);
	}
};
