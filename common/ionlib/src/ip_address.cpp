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
#include "ionlib\ip_address.h"
#include <WinSock2.h>
#include "ionlib\log.h"
#include <Ws2tcpip.h>
namespace ion
{
	IpAddress::IpAddress(const char* ip_address)
	{
		
		int result = inet_pton(AF_INET, ip_address, &this->address_);
		if (result != 1)
		{
			LOGERROR("Unable to initialize ip address from string %s", ip_address);
			this->address_ = INADDR_NONE;
		}
	}
	IpAddress::IpAddress(uint32_t ip_address)
	{
		this->address_ = ip_address;
	}
	uint32_t IpAddress::as_integer()
	{
		return this->address_;
	}
	const char* IpAddress::as_string()
	{
		in_addr address_helper;
		address_helper.s_addr = this->address_;

		return inet_ntop(AF_INET, &address_helper, this->address_string_, sizeof(this->address_string_));
	}
}
