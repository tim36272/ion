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
#include "ionlib/log.h"
namespace ion
{
	double randlf(double low, double high)
	{
		double val = ((double)std::rand() / (double)RAND_MAX)*(high - low) + low;
		LOGSANITY(low <= val && val <= high);
		return val;
	}
	//returns a value in the range [low,high].
	size_t randull(size_t low, size_t high)
	{
		size_t val = (size_t)(((double)std::rand() / (double)(RAND_MAX+1))*((high+1) - low) + low);
		LOGSANITY(low <= val && val <= high);
		return val;
	}
	//uint32_t randul(uint32_t low, uint32_t high)
	//{
	//	uint32_t val;
	//	uint16_t* upper = (uint16_t*)val;
	//	uint16_t* lower = ((uint16_t*)val) + 1;
	//	*upper = std::rand();
	//	*lower = std::rand();
	//	(size_t)(((float)std::rand() / (float)(RAND_MAX + 1))*((high + 1) - low) + low);
	//	LOGSANITY(low <= val && val <= high);
	//	return val;
	//}
	//uint16_t randus(uint16_t low, uint16_t high)
	//{
	//	size_t val = (size_t)(((double)std::rand() / (double)(RAND_MAX + 1))*((high + 1) - low) + low);
	//	LOGSANITY(low <= val && val <= high);
	//	return val;
	//}

	double random_normal_distribution(double mean, double std)
	{
		std::normal_distribution<double> distribution(mean, std);
		std::default_random_engine generator;
		return distribution(generator);
	}
};
