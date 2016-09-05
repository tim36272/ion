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