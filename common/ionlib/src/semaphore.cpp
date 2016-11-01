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
#include "ionlib/semaphore.h"
#include <Windows.h>
#include "ionlib/log.h"
struct semaphore_t
{
	HANDLE semaphore;
};
namespace ion
{
	ion::Semaphore::Semaphore(uint32_t count, uint32_t max)
	{
		semaphore_handle_ = (semaphore_t*)malloc(sizeof(semaphore_t));
		semaphore_handle_->semaphore = CreateSemaphore(NULL, count, max, NULL);
	}

	ion::Semaphore::Semaphore(uint32_t count, uint32_t max, const char* name)
	{
		semaphore_handle_ = (semaphore_t*)malloc(sizeof(semaphore_t));
		semaphore_handle_->semaphore = CreateSemaphore(NULL, count, max, name);
	}

	ion::Error ion::Semaphore::Post()
	{
		LOGASSERT(semaphore_handle_->semaphore != NULL);
		BOOL result = ReleaseSemaphore(semaphore_handle_->semaphore, 1, NULL);	
		if (result != 0)
		{
			return ion::Error::Get(ion::Error::SUCCESS);
		}
		else
		{
			//you could get more deailts with GetLastError
			return ion::Error::Get(ion::Error::UNKNOWN);
		}
	}
	ion::Error ion::Semaphore::Wait(uint32_t timeout)
	{
		LOGASSERT(semaphore_handle_->semaphore != NULL);
		DWORD result;
		if (timeout == TIMEOUT_INFINITE)
		{
			result = WaitForSingleObject(semaphore_handle_->semaphore, INFINITE);
		} else
		{
			result = WaitForSingleObject(semaphore_handle_->semaphore, timeout);
		}
		switch(result) {
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