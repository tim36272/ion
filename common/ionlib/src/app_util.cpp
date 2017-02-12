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
#include "ionlib/app_util.h"
#include "ionlib/log.h"
#ifdef _WIN32
#include <intrin.h>
#include <windows.h>
#endif //_WIN32
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
		AppFail(-1);
	}
	void AppFail(int32_t result)
	{
		LOGINFO("Failed with result %d", result);
		LogFlush();
		fflush(stdout);
		fflush(stderr);
#if !defined(NDEBUG) && defined(_WIN32)
		__debugbreak();
#else
		exit(result);
#endif
	}
	void AppWeakFail(int32_t result)
	{
		LOGINFO("Failed with result %d", result);
		fflush(stdout);
		fflush(stderr);
		//check if the debugger is present
#ifdef _WIN32
		if (IsDebuggerPresent())
		{
			__debugbreak();
		} else
		{
#endif //_WIN32
			//continue
			LOGINFO("Program is continuing because the debugger is not attached");
#ifdef _WIN32
		}
#endif //_WIN32
	}
	void AppSetStrictFloatingPointRules(void)
	{
#ifdef _WIN32
		unsigned int fp_control_state;
		(void)_controlfp_s(&fp_control_state, 0, 0);
		(void)_controlfp_s(&fp_control_state, _EM_INEXACT | _EM_UNDERFLOW, _MCW_EM);
#else //_WIN32
		LOGFATAL("Not supported on this architecture");
#endif //_WIN32
	}
} //namespace ion
