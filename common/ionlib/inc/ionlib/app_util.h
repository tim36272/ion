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
#ifndef ION_APP_UTIL_H_
#define ION_APP_UTIL_H_
#include <stdint.h>
namespace ion
{
	bool AppInit(const char* log_file_name);
	void AppClose();
	void AppFail(int32_t result);
	void AppWeakFail(int32_t result);//only fail if the debugger is attached
	void AppSetStrictFloatingPointRules(void);
} //namespace ion
#endif //IONAPPUTIL_H_
