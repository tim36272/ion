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
#ifndef ION_IP_ADDRESS_H_
#define ION_IP_ADDRESS_H_
#include <stdint.h>
#include "ionlib\byteswap.h"
namespace ion
{
	class IpAddress
	{
	public:
		IpAddress() {}
		IpAddress(const char* ip_address);
		IpAddress(uint32_t ip_address);
		uint32_t as_integer() const; //returns in network byte order
		const char* as_string() const;
		void from_integer(uint32_t ip_address);
		void from_string(const char* ip_address);
	private:
		uint32_t address_;
		char address_string_[16];
	};

	class IpPort
	{
	public:
		IpPort() { }
		enum class Order
		{
			HOST = 0,
			NETWORK = 1
		};
		IpPort(uint16_t port, Order order);
		uint16_t AsNetworkOrder() const;
		uint16_t AsHostOrder() const;
		void FromNetworkOrder(uint16_t port);
		void FromHostOrder(uint16_t port);
		bool operator==(const IpPort& rhs);
	private:
		uint16_t port_; //port in network byte order
	};
}
#endif //ION_IP_ADDRESS_H_
