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
#include "ionlib/serial.h"
#include "ionlib/log.h"
#include <windows.h>
#include <stdio.h>
namespace ion
{
	typedef struct SerialHandle_s
	{
		HANDLE handle;
	} SerialHandle_t;
	Serial::Serial(uint32_t port, ion::Serial::Baud baud)
	{
		handle_open_ = false;
		handle_ = new SerialHandle_t;
		char port_name[16];
		sprintf_s(port_name, "\\\\.\\COM%d", port);
		handle_->handle = CreateFile(port_name, GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
		if (handle_->handle == INVALID_HANDLE_VALUE)
		{
			LPVOID lpMsgBuf;
			DWORD dw = GetLastError();

			FormatMessage(
				FORMAT_MESSAGE_ALLOCATE_BUFFER |
				FORMAT_MESSAGE_FROM_SYSTEM |
				FORMAT_MESSAGE_IGNORE_INSERTS,
				NULL,
				dw,
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
				(LPTSTR)&lpMsgBuf,
				0, NULL);
			LOGFATAL("%s",lpMsgBuf);
		}
		handle_open_ = true;

		DCB serial_params;
		serial_params.DCBlength = sizeof(serial_params);
		if (GetCommState(handle_->handle, &serial_params) == 0)
		{
			LOGFATAL("Error getting serial status");
		}
		switch (baud)
		{
			case ion::Serial::Baud::RATE_300:
				serial_params.BaudRate = CBR_300;
				break;
			case ion::Serial::Baud::RATE_600:
				serial_params.BaudRate = CBR_600;
				break;
			case ion::Serial::Baud::RATE_1200:
				serial_params.BaudRate = CBR_1200;
				break;
			case ion::Serial::Baud::RATE_2400:
				serial_params.BaudRate = CBR_2400;
				break;
			case ion::Serial::Baud::RATE_4800:
				serial_params.BaudRate = CBR_4800;
				break;
			case ion::Serial::Baud::RATE_9600:
				serial_params.BaudRate = CBR_9600;
				break;
			case ion::Serial::Baud::RATE_14400:
				serial_params.BaudRate = CBR_14400;
				break;
			case ion::Serial::Baud::RATE_19200:
				serial_params.BaudRate = CBR_19200;
				break;
			case ion::Serial::Baud::RATE_38400:
				serial_params.BaudRate = CBR_38400;
				break;
			case ion::Serial::Baud::RATE_57600:
				serial_params.BaudRate = CBR_57600;
				break;
			case ion::Serial::Baud::RATE_115200:
				serial_params.BaudRate = CBR_115200;
				break;
			case ion::Serial::Baud::RATE_125200:
				serial_params.BaudRate = 125200;
				break;
			case ion::Serial::Baud::RATE_230400:
				serial_params.BaudRate = 230400;
				break;
			case ion::Serial::Baud::RATE_345600:
				serial_params.BaudRate = 345600;
				break;
			default:
				LOGINFO("Unexpected value for baud: %u, continue at your own risk", baud);
				serial_params.BaudRate = (DWORD)baud;
				break;
		}
		serial_params.ByteSize = 8;
		serial_params.StopBits = ONESTOPBIT;
		serial_params.Parity = NOPARITY;
		//the next four settings match what Arduino expects
		serial_params.fRtsControl = RTS_CONTROL_DISABLE;
		serial_params.fDtrControl = DTR_CONTROL_DISABLE;
		serial_params.XonChar = 0;
		serial_params.XoffChar = 0;
		if (SetCommState(handle_->handle, &serial_params) == 0)
		{
			LOGFATAL("Error setting serial params");
		}

	}
	void Serial::Write(const byte_t* data, uint32_t length)
	{
		DWORD bytes_written;
		BOOL result = WriteFile(handle_->handle, data, length, &bytes_written, NULL);
		if (!result || bytes_written != length)
		{
			LPVOID lpMsgBuf;
			DWORD dw = GetLastError();

			FormatMessage(
				FORMAT_MESSAGE_ALLOCATE_BUFFER |
				FORMAT_MESSAGE_FROM_SYSTEM |
				FORMAT_MESSAGE_IGNORE_INSERTS,
				NULL,
				dw,
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
				(LPTSTR)&lpMsgBuf,
				0, NULL);
			LOGFATAL("%s", lpMsgBuf);
		}
	}
	Serial::~Serial()
	{
		if (handle_open_)
		{
			CloseHandle(handle_->handle);
		}
	}
};