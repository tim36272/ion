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
#ifndef ION_ERROR_H_
#define ION_ERROR_H_
#include <string>
#include <set>
namespace ion
{
	class Error
	{
	public:
		enum status_t
		{
			SUCCESS,
			UNKNOWN,
			PARAMETER,
			PARAMETER_VALUE,
			SOCKET,
			QUEUE_EMPTY,
			//semaphores
			TIMEOUT,
			ABANDONED
		};
		Error() = delete;
		Error(Error::status_t id, std::string explanation);
		static Error Get(Error::status_t status);
		bool operator==(const ion::Error& rhs) const;
		bool operator <(const ion::Error& rhs) const;
	private:
		//This consturctor is private because no one should be allowed to construct an invalid (incomplete) error except the library
		Error(Error::status_t id);
		Error::status_t id_;
		std::string explanation_;
	};
};
#endif //ION_ERROR_H_
