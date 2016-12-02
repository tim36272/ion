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
#include "ionlib\net.h"
#include <WinSock2.h>
#include "ionlib\log.h"
#include <stdint.h>
#include <stdio.h>
namespace ion
{
	int SelectSocket(SOCKET socket_handle, uint32_t timeout_msec)
	{
		fd_set set;
		FD_ZERO(&set);
		FD_SET(socket_handle, &set);
		struct timeval timeout;
		timeout.tv_sec = timeout_msec / 1000;
		timeout.tv_usec = (timeout_msec % 1000) * 1000;

		return select((int)(socket_handle + 1), &set, NULL, NULL, &timeout);
	}
	ion::Error InitSockets()
	{
		//start Windows socket handling and request Winsock Version 2.2
		WSAData wsa_data;
		int result = WSAStartup(MAKEWORD(2, 2), &wsa_data);
		if (result != 0)
		{
			LOGERROR("Failed to initialize socket library with error code %d", result);
			return ion::Error::Get(ion::Error::SOCKET);
		}
		//check that it loaded version 2.2 as requested
		if (LOBYTE(wsa_data.wVersion) != 2 || HIBYTE(wsa_data.wVersion) != 2)
		{
			ion::StopSockets();
			LOGERROR("Failed to initialize socket library. Requested version 2.2, got version %u.%u", static_cast<uint32_t>(HIBYTE(wsa_data.wVersion)), static_cast<uint32_t>(LOBYTE(wsa_data.wVersion)));
			return ion::Error::Get(ion::Error::SOCKET);
		}
		return ion::Error::Get(ion::Error::SUCCESS);
	}
	ion::UdpSocket::UdpSocket()
	{
		this->socket_handle_ = INVALID_SOCKET;
	}
	void StopSockets()
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
	ion::Error UdpSocket::Create()
	{
		this->socket_handle_ = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		if (this->socket_handle_ == INVALID_SOCKET)
		{
			LOGERROR("Failed to create UDP socket, error %u: %s", GetLastError(), ion::getLastErrorString());
			return ion::Error::Get(ion::Error::SOCKET);
		}
		return ion::Error::Get(ion::Error::SUCCESS);
	}
	void UdpSocket::Close()
	{
		closesocket(this->socket_handle_);
	}
	ion::Error UdpSocket::Bind(uint16_t port)
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
			return ion::Error::Get(ion::Error::SOCKET);
		}
		return ion::Error::Get(ion::Error::SUCCESS);
	}
	ion::Error UdpSocket::SendTo(const char* buf, uint32_t len, IpAddress address, uint16_t port)
	{
		sockaddr_in send_to_addr;
		send_to_addr.sin_family = AF_INET;
		send_to_addr.sin_port = htons(port);
		send_to_addr.sin_addr.s_addr = address.as_integer();

		int32_t bytes_sent = sendto(this->socket_handle_, buf, len, 0, (sockaddr*)&send_to_addr, sizeof(sockaddr_in));
		if ((uint32_t)bytes_sent != len)
		{
			LOGERROR("Failed to send packet to %s:%hu, len: %u, error %d: %s", address.as_string(), port, GetLastError(), ion::getLastErrorString());
			return ion::Error::Get(ion::Error::SOCKET);
		}
		return ion::Error::Get(ion::Error::SUCCESS);
	}
	ion::Error UdpSocket::Recv(char* buf, uint32_t len, uint32_t timeout_msec, IpAddress* from_sddress, uint16_t* from_port)
	{
		int result = SelectSocket(socket_handle_, timeout_msec);
		if (result == SOCKET_ERROR)
		{
			//error occurred
			return ion::Error::Get(ion::Error::SOCKET);
		}
		if (result == 0)
		{
			//timeout occurred
			return ion::Error::Get(ion::Error::TIMEOUT);
		} else
		{
			sockaddr_in sender;
			int sender_length = sizeof(sender);
			int32_t bytes = recvfrom(this->socket_handle_, buf, len, 0, (sockaddr*)&sender, &sender_length);
			if (bytes == SOCKET_ERROR)
			{
				LOGERROR("Failed to receive packet: %d %s", GetLastError(), getLastErrorString());
				return ion::Error::Get(ion::Error::SOCKET);
			}

			if (from_sddress)
			{
				from_sddress->from_integer(sender.sin_addr.S_un.S_addr);
			}
			if (from_port)
			{
				*from_port = sender.sin_port;
			}
			return ion::Error::Get(ion::Error::SUCCESS);
		}
	}

	ion::TcpSocket::TcpSocket()
	{
		this->socket_handle_ = INVALID_SOCKET;
	}
	ion::Error ion::TcpSocket::Create(uint16_t port, ion::IpAddress addr)
	{

		sockaddr_.sin_family = AF_INET;
		ion::byteswap((uint8_t*)&port, sizeof(port));
		sockaddr_.sin_port = port;
		sockaddr_.sin_addr.s_addr = addr.as_integer();

		socket_handle_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		LOGASSERT(socket_handle_ != INVALID_SOCKET);
		ion::Error ionresult;
		return ionresult;
	}
	ion::Error ion::TcpSocket::Connect()
	{
		LOGASSERT(socket_handle_ != INVALID_SOCKET);
		int result = connect(socket_handle_, (SOCKADDR*)&sockaddr_, sizeof(sockaddr_));
		LOGASSERT(result != SOCKET_ERROR);
		ion::Error ionresult;
		return ionresult;
	}
	ion::Error ion::TcpSocket::Listen()
	{
		int result = bind(socket_handle_, (LPSOCKADDR)&sockaddr_, sizeof(sockaddr));
		LOGASSERT(result != SOCKET_ERROR);
		listen(socket_handle_, SOMAXCONN);
		ion::Error ionresult;
		return ionresult;
	}
	void ion::TcpSocket::Close()
	{
		LOGASSERT(socket_handle_ != INVALID_SOCKET);
		closesocket(socket_handle_);
	}
	ion::Error ion::TcpSocket::Send(const char* buf, uint32_t len)
	{
		int result;
		result = send(socket_handle_, buf, (int32_t)len, 0);
		if (result != SOCKET_ERROR)
		{
			return ion::Error::Get(ion::Error::SUCCESS);
		} else
		{
			return ion::Error::Get(ion::Error::SOCKET);
		}
	}
	ion::Error ion::TcpSocket::Recv(char* buf, uint32_t buf_len, uint32_t timeout_msec, IpAddress* from_address, uint16_t* from_port, size_t* bytes_read)
	{
		int result = SelectSocket(socket_handle_, timeout_msec);

		if (result == SOCKET_ERROR)
		{
			//error occurred
			return ion::Error::Get(ion::Error::SOCKET);
		}
		if (result == 0)
		{
			//timeout occurred
			return ion::Error::Get(ion::Error::TIMEOUT);
		} else
		{
			//data is available
			sockaddr_in from;
			int from_size = (int)sizeof(from);
			*bytes_read = recvfrom(socket_handle_, buf, buf_len, 0, (sockaddr*)&from, &from_size);
			if (from_address)
			{
				from_address->from_integer(from.sin_addr.S_un.S_addr);
			}
			if (from_port)
			{
				*from_port = from.sin_port;
			}
		}
		return ion::Error::Get(ion::Error::SUCCESS);
	}
} //namespace ion
