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
#include "ionlib/error.h"
#include <set>
#include "ionlib/log.h"

std::set<ion::Error> all_errors = { 
	{ ion::Error::SUCCESS,"No error occurred" },
	{ ion::Error::UNKNOWN,"An unknown error occurred" },
	{ ion::Error::PARAMETER,"Parameter was invalid" },
	{ ion::Error::PARAMETER_VALUE,"Parameter value was out of range" },
	{ ion::Error::SOCKET, "Socket error"},
	{ ion::Error::QUEUE_EMPTY, "Queue was empty" },
	{ ion::Error::QUEUE_FULL, "Queue was full"},
	{ ion::Error::TIMEOUT, "A timeout ocurred" },
	{ ion::Error::ABANDONED, "A mutex was abandoned"}
};

ion::Error::Error(ion::Error::status_t id, std::string explanation)
{
	this->id_ = id;
	this->explanation_ = explanation;
}

ion::Error::Error(ion::Error::status_t id)
{
	this->id_ = id;
}
ion::Error ion::Error::Get(ion::Error::status_t status)
{
	ion::Error temp(status);
	std::set<ion::Error>::iterator it = all_errors.find(temp);
	LOGASSERT(it != all_errors.end());
	return *it;
}

bool ion::Error::operator <(const ion::Error & rhs) const
{
	return (rhs.id_ < this->id_);
}

bool ion::Error::operator==(const ion::Error & rhs) const
{
	return (rhs.id_ == this->id_);
}
bool ion::Error::operator!=(const ion::Error & rhs) const
{
	return (rhs.id_ != this->id_);
}

std::string ion::Error::str() const
{
	return explanation_;
}
