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
#ifndef ION_CONFIG_H_
#define ION_CONFIG_H_
#include <vector>
#include <utility>
#include "ionlib/log.h"

#define CONFIG_KEY_MAX_LENGTH 64
#define CONFIG_VALUE_MAX_LENGTH 128

typedef std::pair<char[CONFIG_KEY_MAX_LENGTH], char[CONFIG_VALUE_MAX_LENGTH]> ConfigItem_t;
namespace ion
{
	class Config
	{
	public:
		Config(const char* filename);

		double Getllf(const char* keyname)
		{
			std::vector<ConfigItem_t>::iterator item = Find(keyname);
			if (strcmp(item->second, "DBL_MAX") == 0)
			{
				return DBL_MAX;
			} else if (strcmp(item->second, "-DBL_MAX") == 0)
			{
				return -DBL_MAX;
			}
			return std::strtod(Find(keyname)->second, NULL);
		}
		float Getf(const char* keyname)
		{
			return std::strtof(Find(keyname)->second, NULL);
		}
		int8_t Getb(const char* keyname)
		{
			return (int8_t)std::strtol(Find(keyname)->second, NULL, 10);
		}
		uint8_t Getub(const char* keyname)
		{
			return (uint8_t)std::strtoul(Find(keyname)->second, NULL, 10);
		}
		int16_t Gets(const char* keyname)
		{
			return (int16_t)std::strtol(Find(keyname)->second, NULL, 10);
		}
		uint16_t Getus(const char* keyname)
		{
			return (uint16_t)std::strtoul(Find(keyname)->second, NULL, 10);
		}
		int32_t Getd(const char* keyname)
		{
			return std::strtol(Find(keyname)->second, NULL, 10);
		}
		uint32_t Getu(const char* keyname)
		{
			return std::strtoul(Find(keyname)->second, NULL, 10);
		}
		int64_t Getdll(const char* keyname)
		{
			return std::strtoll(Find(keyname)->second, NULL, 10);
		}
		uint64_t Getull(const char* keyname)
		{
			return std::strtoull(Find(keyname)->second, NULL, 10);
		}
		const char* Getc(const char* keyname)
		{
			return Find(keyname)->second;
		}



	private:
		std::vector<ConfigItem_t> items;
		std::vector<ConfigItem_t>::iterator Find(const char* keyname)
		{
			for (std::vector<ConfigItem_t>::iterator it = items.begin(); it != items.end(); ++it)
			{
				if (strcmp(it->first, keyname) == 0)
				{
					return it;
				}
			}
			LOGFATAL("Didn't find %s", keyname);
			return items.end();
		}
	};
};
#endif //ION_CONFIG_H_