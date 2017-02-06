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

#define MAX_COMMAND_BUFFER_LENGTH (1024)
namespace ion
{
	void cb_clear(Backdoor* backdoor, std::string args, void* usr_data);

	static const telnet_telopt_t telopts[] = {
		{ TELNET_TELOPT_ECHO,	TELNET_WONT, TELNET_DO },
		{ -1, 0, 0 }
	};
	/* process input line */
	void _online(const char *line, ion::Backdoor *ud)
	{
		/* just a message -- send to all users */
		telnet_printf(ud->telnet_, "%s\n", line);

	}
	void Backdoor::ProcessCommand(const char *buffer,size_t size)
	{
		//check if the sequence is \r\n and if so ignore it:
		if (size == 2 && buffer[0] == '\r' && buffer[1] == '\n')
		{
			this->printf("> ");
			//Prep for next command
			return;
		}
		//strtok modifies the string, so make a copy of it
		char input[MAX_COMMAND_BUFFER_LENGTH];
		memset(input, 0, MAX_COMMAND_BUFFER_LENGTH);
		LOGASSERT(size < MAX_COMMAND_BUFFER_LENGTH);
		memcpy(input, buffer, size);
		//Tokenize the first word out of the string
		char* token_context;
		char* cmd = strtok_s(input, " ", &token_context);
		bool command_found = false;
		//lookup this command in the command table
		for (std::vector<Backdoor::BackdoorCmd>::iterator it = commands_.begin(); it != commands_.end(); ++it)
		{
			if (strncmp(it->name.c_str(), cmd, size) == 0)
			{
				//this is the command, run it
				it->callback(this, token_context, it->usr_data);
				command_found = true;
				break;
			}
		}
		if (!command_found)
		{
			this->printf("Invalid command \"%s\"\n", cmd);
		}

		//size_t i;
		//for (i = 0; i != size; ++i)
		//	linebuffer_push(linebuf, sizeof(linebuf), &linepos,
		//	(char)buffer[i], _online, this);
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
	void telnetEventHandler(telnet_t *, telnet_event_t *ev, void *user_data)
	{
		Backdoor *user = (Backdoor*)user_data;

		switch (ev->type)
		{
			/* data received */
			case TELNET_EV_DATA:
			{
				if (!user->IsConnected())
				{
					return;
				}
				bool in_progress = user->IsCommandInProgress();
				if (!in_progress)
				{
					user->ProcessCommand(ev->data.buffer, ev->data.size);
					//telnet_negotiate(telnet, TELNET_WONT, TELNET_TELOPT_ECHO);
					//telnet_negotiate(telnet, TELNET_WILL, TELNET_TELOPT_ECHO);
					user->FinishCommandHandler();
				} else
				{
					//There is a command in progress, just buffer input for it
					user->AddToInputBuffer(ev->data.buffer, ev->data.size);
				}
				break;
			}
				/* data must be sent */
			case TELNET_EV_SEND:
				if (!user->IsConnected())
				{
					return;
				}
				_send(user->socket_, ev->data.buffer, ev->data.size);
				break;
				/* error */
			case TELNET_EV_ERROR:
				user->socket_.Close();
				telnet_free(user->telnet_);
				user->connected_ = false;
				break;
			default:
				/* ignore */
				break;
		}
	}

	Backdoor::Backdoor(IpPort port)
	{
		port_ = port;
		command_in_progress_ = false;
		connected_ = false;
		Init();
	}
	void Backdoor::Init()
	{
		//add default commands
		AddCommand("help", "Display help menu", &cb_help, nullptr);
		AddCommand("clear", "Clear the screen", &cb_clear, nullptr);
		socket_.Create(port_, IpAddress((uint32_t)INADDR_ANY).as_integer());
		socket_.Listen();
		telnet_ = telnet_init(telopts, telnetEventHandler, 0, this);

		ion::StartThread(&backdoorThread, this);
	}
	void Backdoor::Run()
	{
		//accept a connection
		ion::Error result = socket_.Accept();
		LOGASSERT(result.success());
		connected_ = true;
		//send welcome
		cb_help(this, "", nullptr);
		telnet_printf(telnet_, "\n> ");
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

	///////////////////////////////////////////////////////////////////////////
	//		User-facing interface
	///////////////////////////////////////////////////////////////////////////
	void Backdoor::printf(std::string fmt, ...)
	{
		va_list args;
		va_start(args, fmt);
		telnet_vprintf(telnet_, fmt.c_str(), args);
		va_end(args);
	}

	void Backdoor::newline()
	{
		this->printf("\n");
	}

	void Backdoor::AddCommand(std::string name, std::string help_text, BackdoorCmdCb callback, void* usr_data)
	{
		BackdoorCmd cmd;
		cmd.name = name;
		cmd.help_text = help_text;
		cmd.callback = callback;
		cmd.usr_data = usr_data;
		commands_.push_back(cmd);
	}
	void Backdoor::SetColor(Color color)
	{
		switch (color)
		{
			case Backdoor::Color::BLACK:
				this->printf("\x1B[30m");
				break;
			case Backdoor::Color::RED:
				this->printf("\x1B[31m");
				break;
			case Backdoor::Color::GREEN:
				this->printf("\x1B[32m");
				break;
			case Backdoor::Color::YELLOW:
				this->printf("\x1B[33m");
				break;
			case Backdoor::Color::BLUE:
				this->printf("\x1B[34m");
				break;
			case Backdoor::Color::MAGENTA:
				this->printf("\x1B[35m");
				break;
			case Backdoor::Color::CYAN:
				this->printf("\x1B[36m");
				break;
			case Backdoor::Color::WHITE:
				this->printf("\x1B[37m");
				break;
		}
	}
	void Backdoor::SetBgColor(Color color)
	{
		switch (color)
		{
			case Backdoor::Color::BLACK:
				this->printf("\x1B[40m");
				break;
			case Backdoor::Color::RED:
				this->printf("\x1B[41m");
				break;
			case Backdoor::Color::GREEN:
				this->printf("\x1B[42m");
				break;
			case Backdoor::Color::YELLOW:
				this->printf("\x1B[43m");
				break;
			case Backdoor::Color::BLUE:
				this->printf("\x1B[44m");
				break;
			case Backdoor::Color::MAGENTA:
				this->printf("\x1B[45m");
				break;
			case Backdoor::Color::CYAN:
				this->printf("\x1B[46m");
				break;
			case Backdoor::Color::WHITE:
				this->printf("\x1B[47m");
				break;
		}
	}
	void Backdoor::SetColorHeader()
	{
		SetColor(Color::BLACK);
		SetBgColor(Color::WHITE);
	}
	void Backdoor::SetColorNormal()
	{
		SetColor(Color::WHITE);
		SetBgColor(Color::BLACK);
	}
	void Backdoor::SetCursor(uint32_t line, uint32_t col)
	{
		this->printf("\x1B[%u;%uH",line,col);
	}
	void Backdoor::ClearScreen()
	{
		this->printf("\x1B[2J");
	}
	bool Backdoor::IsCommandInProgress()
	{
		if (command_in_progress_)
		{
			return true;
		} else
		{
			command_in_progress_ = true;
			return false;
		}
	}
	void Backdoor::FinishCommandHandler()
	{
		command_in_progress_ = false;
		//check if the input buffer has a newline in it and if so print the command caret
		if (input_buffer_.size() >= 2)
		{
			if (input_buffer_.front() == '\r')
			{
				input_buffer_.pop_front();
				if (input_buffer_.front() == '\n')
				{
					input_buffer_.pop_front();
					//print the command caret
					this->printf("> ");
				}
			}
		}
	}
	void Backdoor::AddToInputBuffer(const char * buffer, size_t length)
	{
		for (size_t char_index = 0; char_index < length; ++char_index)
		{
			input_buffer_.push_back(buffer[char_index]);
		}
	}
	char Backdoor::GetInput()
	{
		if (input_buffer_.size() == 0)
		{
			//try to get more data
			ion::Error result;
			byte_t buffer[65536];
			size_t bytes_read;
			result = socket_.RecvTimeout(buffer, 65536, 0, nullptr, nullptr, &bytes_read);
			if (result.success())
			{
				telnet_recv(telnet_, buffer, bytes_read);
			}
		}
		if (input_buffer_.size() > 0)
		{
			char val = input_buffer_.front();
			input_buffer_.pop_front();
			return val;
		} else
		{
			return '\0';
		}
	}
	void Backdoor::ClearInputBuffer()
	{
		std::deque<char> empty;
		std::swap(input_buffer_, empty);
		
		//empty automatically goes out of scope
	}
	bool Backdoor::IsConnected()
	{
		return connected_;
	}
	///////////////////////////////////////////////////////////////////////////
	//		Built-in backdoor commands
	///////////////////////////////////////////////////////////////////////////
	void cb_help(Backdoor* backdoor, std::string args, void*)
	{
		//find the longest command
		size_t longest_command = strlen("Command"); 
		for (std::vector<Backdoor::BackdoorCmd>::iterator it = backdoor->commands_.begin(); it != backdoor->commands_.end(); ++it)
		{
			if (it->name.length() > longest_command)
			{
				longest_command = it->name.length();
			}
		}
		backdoor->SetCursor(0, 0);
		backdoor->ClearScreen();
		backdoor->SetBgColor(Backdoor::Color::WHITE);
		backdoor->SetColor(Backdoor::Color::BLACK);
		backdoor->printf("%-*sDescription", longest_command + 3, "Command", "Description");
		backdoor->SetColor(Backdoor::Color::WHITE);
		backdoor->SetBgColor(Backdoor::Color::BLACK);
		backdoor->newline();

		//print the command table
		for (std::vector<Backdoor::BackdoorCmd>::iterator it = backdoor->commands_.begin(); it != backdoor->commands_.end(); ++it)
		{
			backdoor->printf("%-*s%s\n", longest_command+3, it->name.c_str(), it->help_text.c_str());
		}
		backdoor->printf("\n");
	}
	void cb_clear(Backdoor* backdoor, std::string args, void*)
	{
		backdoor->ClearScreen();
		backdoor->SetCursor(0, 0);
	}
};