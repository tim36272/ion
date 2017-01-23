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
#ifndef ION_BACKDOOR_H_
#define ION_BACKDOOR_H_
#include "ionlib\ip_address.h"
#include "ionlib\net.h"
extern "C" {
	struct telnet_t;
	union telnet_event_t;
}
namespace ion
{
	class Backdoor
	{
	public:
		Backdoor(IpPort port);
	private:
		IpPort port_;
		void Init();
		friend void backdoorThread(void* args);
		friend void telnetEventHandler(telnet_t *telnet, telnet_event_t *ev, void *user_data);
		friend void _online(const char *line, ion::Backdoor *ud);
		void Run();
		ion::TcpSocket socket_;
		telnet_t* telnet_;
		void _input(const char *buffer, size_t size);
		char linebuf[256];
		int linepos;

	};
};
#endif //ION_BACKDOOR_H_