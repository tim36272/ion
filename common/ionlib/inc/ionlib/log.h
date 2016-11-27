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
#ifndef ION_LOG_H_
#define ION_LOG_H_
#include <stdint.h>
#include <stdlib.h>
#include "app_util.h"

#define IONLOG_MAX_MESSAGE_LENGTH 512

namespace ion
{
	//define macros for debug, info, warn, error, and fatal types
	bool LogInit(const char* log_file_name);
	void LogPrintf(const char* file, uint32_t line, char* format, ...);
	void LogClose();
	void LogFlush();

#define LOGDEBUG(...) ion::LogPrintf(__FILE__, __LINE__, "DEBUG: " ## __VA_ARGS__)
#define LOGINFO(...)  ion::LogPrintf(__FILE__, __LINE__, "INFO: " ## __VA_ARGS__)
#define LOGWARN(...)  ion::LogPrintf(__FILE__, __LINE__, "WARN: " ## __VA_ARGS__)
#define LOGERROR(...) ion::LogPrintf(__FILE__, __LINE__, "ERROR: " ## __VA_ARGS__)

#define LOGFATAL(...) {\
ion::LogPrintf(__FILE__, __LINE__, "FATAL: " ## __VA_ARGS__);\
ion::AppFail(-1);\
}

	//Assert that a condition is true and halt the program if not
#define LOGASSERT(b,...) {\
  if(!(b)) {\
  LOGFATAL("Assertion failed: "#b" INFO: "##__VA_ARGS__);\
  }\
}

//Assert that a condition is true and print a message, then if the debugger is attached set a breakpoint, otherwise continue running
#define LOGWEAKASSERT(b,...) {\
  if(!(b)) {\
  ion::LogPrintf(__FILE__,__LINE__,"Weak assertion failed: "#b" INFO: "##__VA_ARGS__);\
  ion::AppWeakFail(-1);\
  }\
}

//This is a "true assertion" in the sense that it is a sanity check only, this will/should never happen in reality
#ifdef ION_NO_SANITY_CHECK
#define LOGSANITY(...)
#else
#define LOGSANITY(b,...) {\
  if(!(b)) {\
  LOGFATAL("Sanity check failed: "#b" INFO: "##__VA_ARGS__);\
  }\
}
#endif

} //namespace ion
#endif //IONLOG_H_
