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
#ifndef ION_FILE_H_
#define ION_FILE_H_
namespace ion
{
	template <class T>
	class Point2
	{
	public:
		Point2()
		{
			x1_ = 0;
			x2_ = 0;
		}
		Point2(T x1, T x2)
		{
			x1_ = x1;
			x2_ = x2;
		}
		Point2(const Point2<T> & rhs)
		{
			x1_ = rhs.x1_;
			x2_ = rhs.x2_;
		}
		Point2<T> operator+(const Point2<T>& rhs)
		{
			Point2<T> result;
			result.x1_ = this->x1_ + rhs.x1_;
			result.x2_ = this->x2_ + rhs.x2_;
			return result;
		}
		Point2<T> operator-(const Point2<T>& rhs)
		{
			Point2<T> result;
			result.x1_ = this->x1_ - rhs.x1_;
			result.x2_ = this->x2_ - rhs.x2_;
			return result;
		}
		double norm2()
		{
			double result;
			result = sqrt(this->x1_ * this->x1_ + this->x2_ * this->x2_);
			return result;
		}
		double distance(const Point2<T>& rhs)
		{
			double result;
			double dx = this->x1_ - rhs.x1_;
			double dy = this->x2_ - rhs.x2_;
			result = sqrt(dx*dx + dy*dy);
			return result;
		}
		double distance_sq(const Point2<T>& rhs)
		{
			double result;
			double dx = this->x1_ - rhs.x1_;
			double dy = this->x2_ - rhs.x2_;
			result = dx*dx + dy*dy;
			return result;
		}
		T x1_;
		T x2_;
	};
};
#endif //ION_FILE_H_