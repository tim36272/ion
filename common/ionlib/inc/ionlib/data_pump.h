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
		PumpMessage()
		{
		}
		PumpMessage(uint32_t id_in, T item)
		{
			id = id_in;
			data = std::make_shared<T>(item);
		}
	};

	template <class T>
	class DataPump
	{
	public:
		DataPump()
		{

		}
		void Pump(ion::PumpMessage<T> msg)
		{
			{
				std::lock_guard<std::mutex> lk(mutex_);
				queue_.push(msg);
			}
			cv_.notify_one();

		}
		ion::Error Wait(uint32_t milliseconds, ion::PumpMessage<T>* item)
		{
			std::chrono::duration<double> timeout;
			//get chrono duration
			if (milliseconds == 0)
			{
				timeout = std::chrono::duration<int32_t, std::milli>::max();
			} else
			{
				timeout = std::chrono::duration<int32_t, std::milli>(milliseconds);
			}
			std::unique_lock<std::mutex> lk(mutex_);
			ion::Error result = ion::Error::Get(ion::Error::SUCCESS);
			if (queue_.empty())
			{
				//wait for an item
				cv_.wait_for(lk, timeout, [=]
				{
					return !this->queue_.empty();
				});
			}

			if (queue_.empty())
			{
				result = ion::Error::Get(ion::Error::TIMEOUT);
			} else
			{
				*item = queue_.front();
				queue_.pop();
				result = ion::Error::Get(ion::Error::SUCCESS);
			}
			lk.unlock();
			return result;
		}
	private:
		std::condition_variable cv_;
		std::mutex mutex_;
		std::queue<ion::PumpMessage<T>> queue_;
	};
};
#endif //ION_DATA_PUMP_H_