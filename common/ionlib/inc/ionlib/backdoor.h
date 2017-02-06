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
#include <vector>
#include <deque>
#include <string>
#include "ionlib\ip_address.h"
#include "ionlib\net.h"
#include "ionlib\mutex.h"
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
		void printf(std::string fmt, ...);
		void newline();
		typedef void(*BackdoorCmdCb)(Backdoor* backdoor, std::string args, void* usr_data);
		void AddCommand(std::string name, std::string help_text, BackdoorCmdCb callback, void* usr_data);
		enum class Color
		{
			BLACK,
			RED,
			GREEN,
			YELLOW,
			BLUE,
			MAGENTA,
			CYAN,
			WHITE
		};
		void SetColor(Color color);
		void SetBgColor(Color color);
		void SetColorHeader();
		void SetColorNormal();
		void SetCursor(uint32_t line, uint32_t col);
		void ClearScreen();
		bool IsCommandInProgress();
		void FinishCommandHandler();
		void AddToInputBuffer(const char* buffer, size_t length);
		char GetInput();
		void ClearInputBuffer();
		bool IsConnected();
	private:
		IpPort port_;
		void Init();
		friend void backdoorThread(void* args);
		friend void telnetEventHandler(telnet_t *telnet, telnet_event_t *ev, void *user_data);
		friend void _online(const char *line, ion::Backdoor *ud);
		void Run();
		ion::TcpSocket socket_;
		bool connected_;
		telnet_t* telnet_;
		void ProcessCommand(const char *buffer, size_t size);
		char linebuf[256];
		int linepos;
		//this mutex is locked when a callback is in progress, which generally indicates a free-running
		//	backdoor function is running and thus the input should be directed to that function

		struct BackdoorCmd
		{
			std::string name;
			std::string help_text;
			BackdoorCmdCb callback;
			void* usr_data;
		};
		std::vector<BackdoorCmd> commands_;
		friend void cb_help(Backdoor* backdoor, std::string args, void* usr_data);
		bool command_in_progress_;
		std::deque<char> input_buffer_; //used when a command is in progress so it can get new input

	};
};
#endif //ION_BACKDOOR_H_