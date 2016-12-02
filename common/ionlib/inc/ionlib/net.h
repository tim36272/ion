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
#ifndef ION_NET_H_
#define ION_NET_H_
#include <WinSock2.h>
#include "ionlib\ip_address.h"
#include "ionlib\error.h"
#include "ionlib\byteswap.h"
namespace ion
{
	ion::Error InitSockets();
	void StopSockets();
	class UdpSocket
	{
	public:
		UdpSocket();
		ion::Error Create();
		void Close();
		ion::Error Bind(uint16_t port);
		ion::Error SendTo(const char* buf, uint32_t len, IpAddress address, uint16_t port);
		ion::Error Recv(char* buf, uint32_t len, uint32_t timeout_msec, IpAddress* from_sddress, uint16_t* from_port);
	private:
		SOCKET socket_handle_;
	};
	class TcpSocket
	{
	public:
		TcpSocket();
		ion::Error Create(uint16_t port, ion::IpAddress addr);
		ion::Error Connect();
		ion::Error Listen();
		void Close();
		ion::Error Send(const char* buf, uint32_t len);
		ion::Error Recv(char* buf, uint32_t buf_len, uint32_t timeout_msec, IpAddress* from_address, uint16_t* from_port, size_t* bytes_read);
	private:
		SOCKADDR_IN sockaddr_;
		SOCKET socket_handle_;
	};
} //namespace ion
#endif //IONNET_H_
