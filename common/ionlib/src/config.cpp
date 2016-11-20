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
#include "ionlib/config.h"
#include <fstream>
#include "ionlib/log.h"
namespace ion
{
	Config::Config(const char* filename)
	{
		std::ifstream fin;
		fin.open(filename);
		//parse each line
		ConfigItem_t item;
		bool reading_key = true;
		bool value_read = false;
		uint32_t string_index = 0;
		while (fin.good())
		{
			//read a character at a time, checking for special characters
			char letter;
			fin.get(letter);
			if (!fin.good())
			{
				letter = '\0';
			}
			switch (letter)
			{
				case '#':
					//the rest of the line is a comment. If there was a value being read then push it onto the vector
					if (value_read)
					{
						item.second[string_index] = '\0';
						items.push_back(item);
						reading_key = true;
						value_read = false;
						string_index = 0;
					}
					fin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
					break;
				case '=':
					//switch to reading the value
					LOGASSERT(reading_key && !value_read,"Config key was incorrectly formatted");
					item.first[string_index] = '\0';
					reading_key = false;
					string_index = 0;
					break;
				case ' ':
				case '\t':
					//ignore whitespace
					break;
				case '\n':
				case '\r':
				case '\0':
					if (value_read)
					{
						//end of a value
						item.second[string_index] = '\0';
						items.push_back(item);
						reading_key = true;
						value_read = false;
						string_index = 0;
					}
					break;
				default:
					//all other characters are used in the strings
					if (reading_key)
					{
						item.first[string_index++] = letter;
					} else
					{
						item.second[string_index++] = letter;
						value_read = true;
					}
					break;
			}
		}
	}
};