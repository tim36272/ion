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
#ifndef ION_QUEUE_H_
#define ION_QUEUE_H_
#include <queue>
#include <chrono>
#include <condition_variable>
#include "ionlib/error.h"
#include "ionlib\log.h"
#ifndef NOMINMAX
#define NOMINMAX
#endif

#define QUEUE_NO_BLOCK (uint32_t)(-1)
#define QUEUE_WAIT_FOREVER (uint32_t)(0)

namespace ion
{
	template <class T>
	class Queue
	{
	public:
		enum class FullBehavior
		{
			RING_BUFFER,
			FAIL
		};
		Queue(uint32_t max_capacity = 0, FullBehavior full_behavior = FullBehavior::RING_BUFFER ) : max_capacity_(max_capacity), full_behavior_(full_behavior)
		{

		}
		ion::Error Push(T& data)
		{
			//check if the queue is full
			//dequeue items until the queue is below capacity. Note that this isn't entirely accurate: it is possible for another thread to dequeue an item while this thread is working, so don't depend on this to guarantee a max size
			while(max_capacity_ > 0 && full_behavior_ == FullBehavior::RING_BUFFER && queue_.size() >= max_capacity_)
			{
				ion::Error result = Pop(QUEUE_NO_BLOCK);
				if (!result.success())
				{
					LOGERROR("Queue was full but dequeing an item failed with result \"%s\"", result.str());
				}
			}
			if (max_capacity_ > 0 && queue_.size() >= max_capacity_)
			{
				return ion::Error::Get(ion::Error::QUEUE_FULL);
			}

			{
				std::lock_guard<std::mutex> lk(mutex_);
				queue_.push(data);
			}
			cv_.notify_one();
			return ion::Error::Get(ion::Error::SUCCESS);
		}
		ion::Error Pop(uint32_t milliseconds, T* item = nullptr)
		{
			std::chrono::duration<double> timeout;
			//get chrono duration
			if (milliseconds == QUEUE_WAIT_FOREVER)
			{
				timeout = (std::chrono::duration<int32_t, std::milli>::max)();
			} else
			{
				timeout = std::chrono::duration<int32_t, std::milli>(milliseconds);
			}
			std::unique_lock<std::mutex> lk(mutex_);
			ion::Error result;
			if (queue_.empty() && milliseconds != QUEUE_NO_BLOCK)
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
				if (item)
				{
					*item = queue_.front();
				}
				queue_.pop();
				result = ion::Error::Get(ion::Error::SUCCESS);
			}
			lk.unlock();
			return result;
		}
		size_t size()
		{
			return queue_.size();
		}
	private:
		std::condition_variable cv_;
		std::mutex mutex_;
		std::queue<T> queue_;
		uint32_t max_capacity_;
		FullBehavior full_behavior_;
	};
}
#endif //ION_QUEUE_H_
