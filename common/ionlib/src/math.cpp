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
#include <random>
#include "ionlib/math.h"
namespace ion
{
	double randlf(double low, double high)
	{
		return ((double)std::rand() / (double)RAND_MAX)*(high - low) + low;
	}
	size_t randull(size_t low, size_t high)
	{
		return llround(((double)std::rand() / (double)RAND_MAX)*(high - low) + low);
	}

	double random_normal_distribution(double mean, double std)
	{
		std::normal_distribution<double> distribution(mean, std);
		std::default_random_engine generator;
		return distribution(generator);
	}
};
