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
#ifndef ION_DATA_PUMP_H_
#define ION_DATA_PUMP_H_
#include "ionlib/error.h"
#include "stdint.h"
#include <memory>
#include <queue>
#include <condition_variable>
#include <chrono>
namespace ion
{
	template <class T>
	class PumpMessage
	{
	public:
		uint32_t id;
		std::shared_ptr<T> data;
	};

	template <class T>
	class DataPump
	{
	public:
		DataPump() : lock_(mutex_)
		{

		}
		void Pump(ion::PumpMessage<T>* msg)
		{
			queue_.push(msg);
		}
		ion::Error Wait(std::chrono::duration timeout)
		{
			cv_.wait_for(lock_, timeout, []
			{
				return !queue_.empty();
			});
			if (queue_.empty())
			{
				return 
			}
		}
		ion::Error
	private:
		std::condition_variable cv_;
		std::mutex mutex_;
		std::unique_lock<std::mutex> lock_;
		std::queue<ion::PumpMessage<T>*> queue_;
	};
};
#endif //ION_DATA_PUMP_H_