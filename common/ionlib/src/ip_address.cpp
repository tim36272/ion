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
		
		int result = inet_pton(AF_INET, ip_address, &address_);
		if (result != 1)
		{
			LOGERROR("Unable to initialize ip address from string %s", ip_address);
			address_ = INADDR_NONE;
			return;
		}
		memcpy(address_string_, ip_address, sizeof(address_string_));
	}
	IpAddress::IpAddress(uint32_t ip_address)
	{
		address_ = ip_address;
		in_addr address_helper;
		address_helper.s_addr = address_;
		(void)inet_ntop(AF_INET, &address_helper, address_string_, sizeof(address_string_));
	}
	uint32_t IpAddress::as_integer() const
	{
		return address_;
	}
	const char* IpAddress::as_string() const
	{
		return address_string_;
	}
	void IpAddress::from_integer(uint32_t ip_address)
	{
		address_ = ip_address;
		in_addr address_helper;
		address_helper.s_addr = address_;
		(void)inet_ntop(AF_INET, &address_helper, address_string_, sizeof(address_string_));
	}
	void IpAddress::from_string(const char* ip_address)
	{
		int result = inet_pton(AF_INET, ip_address, &address_);
		if (result != 1)
		{
			LOGERROR("Unable to initialize ip address from string %s", ip_address);
			this->address_ = INADDR_NONE;
			return;
		}
		memcpy(address_string_, ip_address, sizeof(address_string_));
	}

	IpPort::IpPort(uint16_t port, IpPort::Order order)
	{
		if (order == IpPort::Order::HOST)
		{
			ion::byteswap((uint8_t*)&port, 2);
		}
		port_ = port;
	}
	void IpPort::FromNetworkOrder(uint16_t port)
	{
		port_ = port;
	}
	void IpPort::FromHostOrder(uint16_t port)
	{
		ion::byteswap((uint8_t*)&port, 2);
		port_ = port;
	}
	bool IpPort::operator==(const IpPort & rhs)
	{
		return port_ == rhs.port_;
	}
	uint16_t IpPort::AsNetworkOrder() const
	{
		return port_;
	}
	uint16_t IpPort::AsHostOrder() const
	{
		uint16_t port_host = port_;
		ion::byteswap((uint8_t*)&port_host, 2);
		return port_host;
	}
}
