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
#ifndef SENTRY_OUTPUT_H_
#define SENTRY_OUTPUT_H_
#include "ionlib\iondef.h"
#include "ionlib\queue.h"
#include "ionlib\timer.h"
#include "ffwrapper/write.h"
#include "sentry\Common.h"
namespace ion
{
	struct CameraOutputConfig_t
	{
		CameraOutputConfig_t(uint32_t prerecord_frames) : video_file_open_(false), image_input_(prerecord_frames)
		{
		}
		ion::Queue<std::shared_ptr<ion::Image>> image_input_;
		ion::Queue<ion::EventBase> event_input_;
		ion::Timer fps_;
		ion::FFWriter* writer;
		bool video_file_open_;
		std::string output_dir_;
		std::string camera_name_;

		//debug/instrumentation
	};

	void cameraOutputThread(void* args);
};
#endif //SENTRY_OUTPUT_H_