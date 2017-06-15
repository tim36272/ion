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
#ifndef SENTRY_CAMERA_IO_H_
#define SENTRY_CAMERA_IO_H_
#include <vector>
#include "ionlib/timer.h"
#include "ionlib/queue.h"
#include "ffwrapper/read.h"
namespace ion
{
	struct CameraIoConfig_t
	{
		CameraIoConfig_t(std::string camera_uri) : reader(camera_uri)
		{
			shutdownInProgress = false;
		}
		ion::FFReader reader;
		ion::Timer fps_;
		std::vector<ion::Queue<std::shared_ptr<ion::Image>>*> output; //the thread pushes each new image onto each queue in this vector
		bool shutdownInProgress;
	};
	void cameraIoThread(void* args);
}
#endif //CAMERA_IO_H_