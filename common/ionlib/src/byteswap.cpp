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
#include "ionlib/byteswap.h"
#include "ionlib/log.h"
namespace ion
{
	void byteswap(uint8_t* val, uint8_t bytes)
	{
		if (bytes == 1)
		{
			return;
		}
		LOGASSERT(bytes % 2 == 0);
		for (uint8_t byte_index = 0; byte_index < bytes / 2U; ++byte_index)
		{
			uint8_t temp = val[byte_index];
			val[byte_index] = val[bytes - byte_index - 1];
			val[bytes - 1 - byte_index] = temp;
		}
	}
}