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
#ifndef ION_MUTEX_H_
#define ION_MUTEX_H_
#include <stdint.h>
#include "ionlib/error.h"
#include "ionlib/multithread.h"
struct mutex_t;
namespace ion
{
	class Mutex
	{
	public:
		Mutex(bool init_locked = false);
		Mutex(bool init_locked, const char* name);
		ion::Error Lock(uint32_t timeout = TIMEOUT_INFINITE);
		ion::Error Unlock(); //timeout in milliseconds
	private:
		//this is a PIMPL (http://stackoverflow.com/questions/60570/why-should-the-pimpl-idiom-be-used) for cross-platform portability
		mutex_t* mutex_handle_;
	};
};
#endif //ION_MUTEX_H_