#include "ionlib\net.h"
#include <WinSock2.h>
#include "ionlib\log.h"
#include <stdint.h>
#include <stdio.h>
namespace ion
{
	bool ion::InitSockets()
	{
		//start Windows socket handling and request Winsock Version 2.2
		WSAData wsa_data;
		int result = WSAStartup(MAKEWORD(2, 2), &wsa_data);
		if (result != 0)
		{
			LOGERROR("Failed to initialize socket library with error code %d", result);
			return false;
		}
		//check that it loaded version 2.2 as requested
		if (LOBYTE(wsa_data.wVersion) != 2 || HIBYTE(wsa_data.wVersion) != 2)
		{
			ion::StopSockets();
			LOGERROR("Failed to initialize socket library. Requested version 2.2, got version %u.%u", static_cast<uint32_t>(HIBYTE(wsa_data.wVersion)), static_cast<uint32_t>(LOBYTE(wsa_data.wVersion)));
			return false;
		}
		return true;
	}
	void ion::StopSockets()
	{
		WSACleanup();
	}

	const char* getLastErrorString()
	{
		DWORD wsa_error = GetLastError();
		LPVOID error_message_buffer;
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, wsa_error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&error_message_buffer, 0, NULL);
		return (const char*)error_message_buffer;
	}
	bool UdpSocket::Create()
	{
		this->socket_handle_ = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		if (this->socket_handle_ == INVALID_SOCKET)
		{
			LOGERROR("Failed to create UDP socket, error %u: %s", GetLastError(), ion::getLastErrorString());
			return false;
		}
		return true;
	}
	void UdpSocket::Close()
	{
		closesocket(this->socket_handle_);
	}
	bool UdpSocket::Bind(uint16_t port)
	{
		return false;
	}
	bool UdpSocket::SendTo(const char* buf, uint32_t len, IpAddress address, uint16_t port)
	{
		return false;
	}
	int32_t UdpSocket::Recv(char* buf, uint32_t len, IpAddress* src_sddress)
	{
		return -1;
	}
} //namespace ion