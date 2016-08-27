#include "ionlib\ip_address.h"
#include <WinSock2.h>
#include "ionlib\log.h"
namespace ion
{
	IpAddress::IpAddress(const char* ip_address)
	{
		this->address_ = inet_addr(ip_address);
		if (!this->address_)
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
		return inet_ntoa(address_helper);
	}
}