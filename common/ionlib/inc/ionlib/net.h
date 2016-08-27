#ifndef ION_NET_H_
#define ION_NET_H_
#include <WinSock2.h>
#include "ionlib\ip_address.h"
namespace ion
{
	bool InitSockets();
	void StopSockets();
	class UdpSocket
	{
	public:
		bool Create();
		void Close();
		bool Bind(uint16_t port);
		bool SendTo(const char* buf, uint32_t len, IpAddress address, uint16_t port);
		int32_t Recv(char* buf, uint32_t len, IpAddress* src_sddress);
	private:
		SOCKET socket_handle_;
	};
} //namespace ion
#endif //IONNET_H_