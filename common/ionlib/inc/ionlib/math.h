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

	double randlf(double low, double high);
	size_t randull(size_t low, size_t high);
	double random_normal_distribution(double low, double high);
};
#endif //ION_MATH_H_