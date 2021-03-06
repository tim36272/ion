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
#ifndef SENTRY_COMMON_H_
#define SENTRY_COMMON_H_
namespace ion
{
	class EventBase
	{
	public:
		enum class Type
		{
			EVENT_STATUS,
			CAMERA_MOTION,
			PIR
		};
		Type event_type_;
		bool in_progress_;
	};
};
#endif //SENTRY_COMMON_H_