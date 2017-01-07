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
#ifndef ION_DOUBLE_BUFFER_H_
#define ION_DOUBLE_BUFFER_H_
#include <stdint.h>
namespace ion
{
	template <class T>
	class ReliableDoubleBuffer
	{
	public:
		ReliableDoubleBuffer<T>(uint32_t num_receivers)
		{

		}
	private:
		T data_[2];
	};
};
#endif //ION_DOUBLE_BUFFER_H_