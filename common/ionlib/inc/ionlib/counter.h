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
#ifndef ION_COUNTER_H_
#define ION_COUNTER_H_
#include <limits>
#include <stdint.h>
namespace ion
{
	template <class T>
	class Counter
	{
	public:
		Counter()
		{
			Reset();
		}
		void Reset()
		{
			sum_ = static_cast<T>(0);
			last_ = static_cast<T>(0);
			sum_sq_ = static_cast<T>(0);
			min_ = std::numeric_limits<T>::max();
			max_ = std::numeric_limits<T>::lowest();
			count_ = 0;
		}
		void Add(T meas)
		{
			count_++;
			sum_ += meas;
			last_ = meas;
			sum_sq_ += meas*meas;
			if (meas < min_)
			{
				min_ = meas;
			} else if (meas > max_)
			{
				max_ = meas;
			}
		}
		T GetVariance()
		{
			return (sum_sq_ - (sum_*sum_) / count_) / (count_ - 1);
		}
		T GetStd()
		{
			return sqrt(GetVariance());
		}
		double GetMean()
		{
			return sum_ / (double)count_;
		}
		T GetLast()
		{
			return last_;
		}
		T GetMin()
		{
			return min_;
		}
		T GetMax()
		{
			return max_;
		}
	private:
		T sum_;
		T last_;
		T sum_sq_;
		T min_;
		T max_;
		uint64_t count_;
	};
};
#endif //ION_COUNTER_H_