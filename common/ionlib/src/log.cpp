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
#include "ionlib\log.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "ionlib\app_util.h"
#include "ionlib\time.h"

namespace ion
{
	FILE* g_log_file = NULL;

	bool LogInit(const char* log_file_name)
	{
		errno_t result = fopen_s(&g_log_file, log_file_name, "w");
		if (result != 0 || g_log_file == NULL)
		{
			//bypass the normal logger and just printf since logging isn't initialized yet
			printf("Failed to open log file %s", log_file_name);
			ion::AppFail(-1);
			return false;
		}
		//test the log
		LOGINFO("Logging to %s", log_file_name);
		return true;
	}
	void LogClose()
	{
		if (g_log_file != NULL)
		{
			fclose(g_log_file);
		}
	}
	void LogPrintf(const char* file, uint32_t line, char* format, ...)
	{
		va_list args;
		va_start(args, format);
		char buffer[IONLOG_MAX_MESSAGE_LENGTH];
		//advance the file name pointer to the letter right after the last slash
		const char* cursor = file;
		while (*cursor != '\0')
		{
			if (*cursor == '\\')
			{
				file = cursor + 1;
			}
			cursor++;
		}
		int num_bytes_written = snprintf(buffer, IONLOG_MAX_MESSAGE_LENGTH, "%9.3lf %s:%d ", ion::TimeGet()/1000.0, file, line);
		num_bytes_written += vsnprintf(buffer + num_bytes_written, IONLOG_MAX_MESSAGE_LENGTH - num_bytes_written, format, args);
		num_bytes_written += snprintf(buffer + num_bytes_written, IONLOG_MAX_MESSAGE_LENGTH - num_bytes_written, "\r\n");
		printf(buffer);
		fwrite(buffer, 1, num_bytes_written, g_log_file);
		va_end(args);
	}

	void LogFlush()
	{
		fflush(g_log_file);
	}
} //namespace ion