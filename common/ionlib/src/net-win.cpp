#include "ionlib\net.h"
#include <WinSock2.h>
#include "ionlib\log.h"
#include <stdint.h>
#include <stdio.h>
namespace ion
{
	ion::UdpSocket::UdpSocket()
	{
		this->socket_handle_ = INVALID_SOCKET;
	}
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
		int32_t status;
		sockaddr_in addr;
		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = INADDR_ANY;
		addr.sin_port = htons(port);

		status = bind(this->socket_handle_, (const sockaddr*)&addr, sizeof(sockaddr_in));
		if (status < 0)
		{
			LOGERROR("Failed to bind socket, error %u: %s", GetLastError(), ion::getLastErrorString());
			return false;
		}
		return true;;
	}
	bool UdpSocket::SendTo(const char* buf, uint32_t len, IpAddress address, uint16_t port)
	{
		sockaddr_in send_to_addr;
		send_to_addr.sin_family = AF_INET;
		send_to_addr.sin_port = htons(port);
		send_to_addr.sin_addr.s_addr = address.as_integer();

		int32_t bytes_sent = sendto(this->socket_handle_, buf, len, 0, (sockaddr*)&send_to_addr, sizeof(sockaddr_in));
		if (bytes_sent != len)
		{
			LOGERROR("Failed to send packet to %s:%hu, len: %u, error %d: %s", address.as_string(), port, GetLastError(), ion::getLastErrorString());
			return false;
		}
		return true;
	}
	int32_t UdpSocket::Recv(char* buf, uint32_t len, IpAddress* src_sddress)
	{
		return -1;
	}
} //namespace ion