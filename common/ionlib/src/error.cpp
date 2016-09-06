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
#include "ionlib\error.h"
#include <map>
#include "ionlib\log.h"

std::map<ion::Error::status_t,ion::Error> all_errors = { 
	{ion::Error::SUCCESS,{ ion::Error::SUCCESS,"No error occurred" }},
	{ ion::Error::VALUE_ERROR,{ ion::Error::VALUE_ERROR,"No error occurred" } }
};

ion::Error ion::Error::Get(ion::Error::status_t status)
{
	std::map<ion::Error::status_t,ion::Error>::iterator it = all_errors.find(status);
	LOGASSERT(it != all_errors.end());
	return it->second;
}