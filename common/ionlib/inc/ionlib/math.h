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
#ifndef ION_MATH_H_
#define ION_MATH_H_
namespace ion
{
#ifndef PI
#define PI 3.1415926535897
#endif
#ifndef NATURAL_NUMBER
#define NATURAL_NUMBER 2.71828182845904523536028747135266249775724709369995
#endif
#ifndef MAX
#define MAX(a,b)  (((a) > (b)) ? (a) : (b))
#endif

#ifndef MIN
#define MIN(a,b)  (((a) < (b)) ? (a) : (b))
#endif

#define FEET_TO_METERS (0.3048)

	double randlf(double low, double high);
	uint64_t randull(uint64_t low, uint64_t high);
	double random_normal_distribution(double low, double high);

	template <class T>
	inline T Add(T left, T right)
	{
		return left + right;
	}
	template <class T>
	inline T Subtract(T left, T right)
	{
		return left - right;
	}
	template <class T>
	inline T SubtractSwap(T left, T right)
	{
		return right - left;
	}
	template <class T>
	inline T Multiply(T left, T right)
	{
		return left * right;
	}
	template <class T>
	inline T Divide(T left, T right)
	{
		return left / right;
	}
	template <class T>
	inline T SwapDivide(T left, T right)
	{
		return right / left;
	}
	template <class T>
	inline T Exp(T in)
	{
		return static_cast<T>(expf(static_cast<float>(in)));
	}
	template <class T>
	inline T Log(T in)
	{
		LOGASSERT(in > static_cast<T>(0));
		return static_cast<T>(logf(static_cast<float>(in)));
	}
	template <class T>
	inline T LogPerturb(T in)
	{
		if (in > static_cast<T>(0))
		{
			return static_cast<T>(logf(static_cast<float>(in)));
		} else if (in == static_cast<T>(0))
		{
			return static_cast<T>(logf(0.00000000001f));
		}
		LOGFATAL("Cannot take the log of a negative number %f", in);
		return static_cast<T>(0);
	}
	//returns 0 for false and 1 for true (ion::Matrix::operator== depends on this behavior)
	template <class T>
	inline uint32_t Compare(T left, T right)
	{
		return (left == right) ? 1 : 0;
	}
	//derivative of tanh function
	template <class T>
	inline T derivative_tanh(T in)
	{
		float temp = std::tanhf(static_cast<float>(in);
		return static_cast<T>(1.0 - temp*temp));
	}
	template <class T>
	inline bool Equal(T lhs, T rhs)
	{
		return lhs == rhs;
	}
	template <class T>
	inline T clamp(T val, T min, T max)
	{
		if (val < min)
		{
			return min;
		} else if (val > max)
		{
			return max;
		}
		return val;
	}
};
#endif //ION_MATH_H_
