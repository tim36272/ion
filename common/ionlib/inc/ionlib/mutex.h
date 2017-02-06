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
		Mutex(bool init_locked = false, const char* name = nullptr);
		/* Lock the mutex
		@param timeout	time to wait, in milliseconds, for the lock. Passing 0 causes the function to return immediately with or without lock
		@return			ion::Error::ABANDONED	if a now dead processes/thread owned the lock. The state of the mutex is indeterminent
						ion::Error::SUCCESS		The caller now has the mutex
						ion::Error::TIMEOUT		The mutex was not acquired in time. The caller does not have the mutex.
						ion::Error::UNKNOWN		Unknown error occurred, in Windows call GetLastError for details
		*/
		ion::Error Lock(uint32_t timeout = TIMEOUT_INFINITE);
		ion::Error Unlock(); //timeout in milliseconds
	private:
		//this is a PIMPL (http://stackoverflow.com/questions/60570/why-should-the-pimpl-idiom-be-used) for cross-platform portability
		mutex_t* mutex_handle_;
	};
};
#endif //ION_MUTEX_H_
