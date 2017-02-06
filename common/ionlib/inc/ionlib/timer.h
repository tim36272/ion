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
#ifndef ION_TIMER_H_
#define ION_TIMER_H_
#include <stdint.h>
#include "ionlib/time.h"
#include <float.h>
#include <math.h>
namespace ion
{
	class Timer
	{
	public:
		Timer()
		{
			Reset();
		}
		void Begin()
		{
			start_ = ion::TimeGet();
			timer_running_ = true;
		}
		void End()
		{
			count_++;
			last_ = ion::TimeGet() - start_;
			//compute sum
			sum_ += last_;
			//compute variance
			sum_sq_ = sum_sq_ + last_*last_;
			//compute min/max
			if (last_ > max_)
			{
				max_ = last_;
			}
			if (last_ < min_)
			{
				min_ = last_;
			}
			timer_running_ = false;
		}
		void PeriodBegin()
		{
			if (timer_running_)
			{
				End();
			}
			Begin();
		}
		double GetVariance()
		{
			return (sum_sq_ - (sum_*sum_) / count_) / (count_ - 1);
		}
		double GetStd()
		{
			return sqrt(GetVariance());
		}
		double GetMean()
		{
			return sum_ / count_;
		}
		double GetLast()
		{
			return last_;
		}
		double GetMin()
		{
			return min_;
		}
		double GetMax()
		{
			return max_;
		}
		double GetStart()
		{
			return start_;
		}
		void Reset()
		{
			count_ = 0ULL;
			sum_ = 0.0;
			start_ = 0.0;
			sum_sq_ = 0.0;
			last_ = 0.0;
			min_ = DBL_MAX;
			max_ = -DBL_MAX;
			timer_running_ = false;
		}
	private:
		double sum_;
		double start_;
		double last_;
		double sum_sq_;
		double min_;
		double max_;
		uint64_t count_;
		bool timer_running_;
	};
};
#endif //ION_TIMER_H_