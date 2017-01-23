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
#include "ionlib\backdoor.h"
#include "ionlib\thread.h"
#include "ionlib\log.h"
#include "ionlib\iondef.h"
#include "libtelnet.h"
namespace ion
{
	static const telnet_telopt_t telopts[] = {
		{ TELNET_TELOPT_ECHO,	TELNET_WONT, TELNET_DO },
		{ -1, 0, 0 }
	};
	static void linebuffer_push(char *buffer, size_t size, int *linepos,
								char ch, void(*cb)(const char *line, ion::Backdoor *ud),
								ion::Backdoor *ud)
	{

		/* CRLF -- line terminator */
		if (ch == '\n' && *linepos > 0 && buffer[*linepos - 1] == '\r')
		{
			/* NUL terminate (replaces \r in buffer), notify app, clear */
			buffer[*linepos - 1] = 0;
			cb(buffer, ud);
			*linepos = 0;

			/* CRNUL -- just a CR */
		} else if (ch == 0 && *linepos > 0 && buffer[*linepos - 1] == '\r')
		{
			/* do nothing, the CR is already in the buffer */

			/* anything else (including technically invalid CR followed by
			* anything besides LF or NUL -- just buffer if we have room
			* \r
			*/
		} else if (*linepos != size)
		{
			buffer[(*linepos)++] = ch;

			/* buffer overflow */
		} else
		{
			/* terminate (NOTE: eats a byte), notify app, clear buffer */
			buffer[size - 1] = 0;
			cb(buffer, ud);
			*linepos = 0;
		}
	}
	/* process input line */
	void _online(const char *line, ion::Backdoor *ud)
	{
		/* just a message -- send to all users */
		telnet_printf(ud->telnet_, "%s\n", line);

	}
	void Backdoor::_input(const char *buffer,size_t size)
	{
		size_t i;
		for (i = 0; i != size; ++i)
			linebuffer_push(linebuf, sizeof(linebuf), &linepos,
			(char)buffer[i], _online, this);
	}
	static void _send(ion::TcpSocket socket, const char *buffer, size_t size)
	{

		/* send data */
		while (size > 0)
		{
			if(!socket.Send(buffer, (uint32_t)size).success())
			{
				if (errno != EINTR && errno != ECONNRESET)
				{
					LOGFATAL("send() failed: %d\n", errno);
				} else
				{
					return;
				}
			}

			/* update pointer and size to see if we've got more to send */
			buffer += size;
			size -= size;
		}
	}
	void telnetEventHandler(telnet_t *telnet, telnet_event_t *ev, void *user_data)
	{
		Backdoor *user = (Backdoor*)user_data;

		switch (ev->type)
		{
			/* data received */
			case TELNET_EV_DATA:
				user->_input(ev->data.buffer, ev->data.size);
				//telnet_negotiate(telnet, TELNET_WONT, TELNET_TELOPT_ECHO);
				//telnet_negotiate(telnet, TELNET_WILL, TELNET_TELOPT_ECHO);
				break;
				/* data must be sent */
			case TELNET_EV_SEND:
				_send(user->socket_, ev->data.buffer, ev->data.size);
				break;
				/* enable compress2 if accepted by client */
			case TELNET_EV_DO:
				if (ev->neg.telopt == TELNET_TELOPT_COMPRESS2)
					telnet_begin_compress2(telnet);
				break;
				/* error */
			case TELNET_EV_ERROR:
				user->socket_.Close();
				telnet_free(user->telnet_);
				break;
			default:
				/* ignore */
				break;
		}
	}

	Backdoor::Backdoor(IpPort port)
	{
		port_ = port;
		Init();
	}
	void Backdoor::Init()
	{
		socket_.Create(port_, IpAddress((uint32_t)INADDR_ANY).as_integer());
		socket_.Listen();
		ion::StartThread(&backdoorThread, this);
	}
	void Backdoor::Run()
	{
		//accept a connection
		ion::Error result = socket_.Accept();
		LOGASSERT(result.success());
		telnet_ = telnet_init(telopts, telnetEventHandler, 0, this);
		telnet_printf(telnet_, "Welcome to backdoor");

		byte_t buffer[65536];
		size_t bytes_read;
		while (true)
		{
			result = socket_.Recv(buffer, 65536, nullptr, nullptr, &bytes_read);
			if (result.success())
			{
				telnet_recv(telnet_, buffer, bytes_read);
			}
		}
	}
	void backdoorThread(void* args)
	{
		Backdoor* backdoor = (Backdoor*)args;
		backdoor->Run();
	}
};