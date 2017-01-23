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
#include <windows.h>
#include <VersionHelpers.h>
namespace ion
{
	static long long counter_frequency = 0LL;
	static int64_t counter_origin = 0;
	//will be called by backend if the user doesn't call it, you can call this manually if you care about the first measurement being as accurate as possible
	void TimeInit()
	{
		LARGE_INTEGER frequency;
		QueryPerformanceFrequency(&frequency);
		counter_frequency = frequency.QuadPart;
		LARGE_INTEGER counter;
		BOOL result = 0;
		while (result == 0)
		{
			result = QueryPerformanceCounter(&counter);
		}
		counter_origin = counter.QuadPart;
	}
	//fast-query timer, may be the number of seconds since the program started
	double TimeGet()
	{
		if (counter_frequency == 0.0)
		{
			TimeInit();
		}
		LARGE_INTEGER counter;
		QueryPerformanceCounter(&counter);
		long long time_microseconds = ((counter.QuadPart-counter_origin) * 1000000 / counter_frequency);
		//report time in seconds
		return time_microseconds / 1000000.0;

	}
	//gives time since January 1, 1970 at midnight
	double TimeGetEpoch()
	{
		FILETIME time;
//#if NTDDI_VERSION < NTDDI_WIN8 
		GetSystemTimeAsFileTime(&time);
//#else
//		GetSystemTimePreciseAsFileTime(&time);
//#endif
		static const uint64_t EPOCH_DIFFERENCE_TENTH_MICROS = 116444736000000000ull;
		// First convert 100-ns intervals to microseconds, then adjust for the
		// epoch difference
		uint64_t total_us = (((uint64_t)time.dwHighDateTime << 32) | (uint64_t)time.dwLowDateTime);
		total_us -= EPOCH_DIFFERENCE_TENTH_MICROS;

		return total_us / 10000000.0;
	}
	void ThreadSleep(uint32_t milliseconds)
	{
		Sleep(milliseconds);
	}
};
