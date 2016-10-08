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
#ifndef ION_STATE_MACHINE_H_
#define ION_STATE_MACHINE_H_
#include <stdint.h>
#include <vector>
#include "ionlib/error.h"
namespace ion
{
	class State
	{
	public:
		virtual ion::Error Initialize() = 0;
		virtual ion::Error Spin() = 0;
		virtual ion::Error TransitionTo(ion::State* state) = 0;
		virtual ion::Error TransitionFrom(ion::State* state) = 0;
	private:
		uint32_t id_;
	};
	class StateMachine
	{
	public:
		virtual ion::Error ChangeState(ion::State* state) = 0;
		virtual ion::State* GetState()
		{
			return current_state_;
		}
	private:
		std::vector<ion::State> states_;
		ion::State* current_state_;
	};
};
#endif //ION_STATE_MACHINE_H_