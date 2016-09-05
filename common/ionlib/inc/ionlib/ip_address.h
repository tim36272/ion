#ifndef ION_IP_ADDRESS_H_
#define ION_IP_ADDRESS_H_
#include <stdint.h>
namespace ion
{
	class IpAddress
	{
	public:
		IpAddress() {}
		IpAddress(const char* ip_address);
		IpAddress(uint32_t ip_address);
		uint32_t as_integer(); //returns in network byte order
		const char* as_string();
	private:
		uint32_t address_;
		char address_string_[16];
	};
}
#endif //ION_IP_ADDRESS_H_