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
#ifndef ION_TIME_H_
#define ION_TIME_H_
#include <stdint.h>
namespace ion
{
	void TimeInit(); //will be called by backend if the user doesn't call it, you can call this manually if you care about the first measurement being as accurate as possible
	double TimeGet(); //fast-query timer, may be the number of seconds since the program started
	double TimeGetEpoch(); //time that gives time since January 1, 1970 at midnight

	void ThreadSleep(uint32_t milliseconds);

}
#endif //ION_TIME_H_
