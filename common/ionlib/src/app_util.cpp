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
#include "ionlib\app_util.h"
#include "ionlib\log.h"
#include "ionlib\net.h"
#include <intrin.h>
#include <stdio.h>
namespace ion
{
	bool AppInit(const char* log_file_name)
	{
		bool result = true;
		if (!ion::LogInit(log_file_name))
		{
			result = false;
		}
		return result;
	}
	void AppClose()
	{
		ion::LogClose();
	}
	void AppFail(int32_t result)
	{
		ion::LogClose();
#ifndef NDEBUG
		__debugbreak();
#endif
		fflush(stdout);
		fflush(stderr);
		exit(result);
	}
} //namespace ion