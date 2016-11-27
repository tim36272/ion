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

#include "ionlib/mutex.h"
#include <Windows.h>
#include "ionlib/log.h"
struct mutex_t
{
	HANDLE mutex;
};
namespace ion
{
	ion::Mutex::Mutex(bool init_locked)
	{
		mutex_handle_ = (mutex_t*)malloc(sizeof(mutex_t));
		mutex_handle_->mutex = CreateMutex(NULL, init_locked, NULL);
	}

	ion::Mutex::Mutex(bool init_locked, const char* name)
	{
		mutex_handle_ = (mutex_t*)malloc(sizeof(mutex_t));
		mutex_handle_->mutex = CreateMutex(NULL, init_locked,name);
	}

	ion::Error ion::Mutex::Unlock()
	{
		LOGASSERT(mutex_handle_->mutex != NULL);
		BOOL result = ReleaseMutex(mutex_handle_->mutex);
		if (result != 0)
		{
			return ion::Error::Get(ion::Error::SUCCESS);
		} else
		{
			//you could get more deailts with GetLastError
			return ion::Error::Get(ion::Error::UNKNOWN);
		}
	}
	ion::Error ion::Mutex::Lock(uint32_t timeout)
	{
		LOGASSERT(mutex_handle_->mutex != NULL);
		DWORD result;
		if (timeout == TIMEOUT_INFINITE)
		{
			result = WaitForSingleObject(mutex_handle_->mutex, INFINITE);
		} else
		{
			result = WaitForSingleObject(mutex_handle_->mutex, timeout);
		}
		switch (result)
		{
			case WAIT_ABANDONED:
				return ion::Error::Get(ion::Error::ABANDONED);
				break;
			case WAIT_OBJECT_0:
				return ion::Error::Get(ion::Error::SUCCESS);
				break;
			case WAIT_TIMEOUT:
				return ion::Error::Get(ion::Error::TIMEOUT);
				break;
			case WAIT_FAILED: //you could get more deailts with GetLastError
			default:
				return ion::Error::Get(ion::Error::UNKNOWN);
				break;
		}
	}
};
