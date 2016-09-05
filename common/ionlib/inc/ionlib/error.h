/*
This file is part of Ionlib.

Ionlib is free software : you can redistribute it and / or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Ionlib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Foobar.If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef ION_FILE_H_
#define ION_FILE_H_
#include <string>
namespace ion
{
	class Error
	{
	public:
		enum status_t
		{
			SUCCESS,
			VALUE_ERROR,
		};
		Error() = delete;
		Error(Error::status_t id_, std::string explanation_);
		static Error Get(Error::status_t status);
	private:
		Error::status_t id_;
		std::string explanation_;
	};
};
#endif //ION_FILE_H_