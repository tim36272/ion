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
#ifndef ION_SERIAL_H_
#define ION_SERIAL_H_
#include "ionlib\iondef.h"
namespace ion
{
	typedef struct SerialHandle_s SerialHandle_t;
	class Serial
	{
	public:
		enum class Baud
		{
			RATE_300,
			RATE_600,
			RATE_1200,
			RATE_2400,
			RATE_4800,
			RATE_9600,
			RATE_14400,
			RATE_19200,
			RATE_38400,
			RATE_57600,
			RATE_115200
		};
		Serial(uint32_t port, ion::Serial::Baud baud);
		void Write(const byte_t* data, uint32_t length);
		~Serial();

	private:
		SerialHandle_t* handle_;
		bool handle_open_;
	};
};
#endif