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
#ifndef SENTRY_TIMELAPSE_H_
#define SENTRY_TIMELAPSE_H_
#include "ionlib\queue.h"
#include "ffwrapper\Common.h"
#include "sentry\Common.h"
namespace ion
{
	struct TimelapseConfig_t
	{
		TimelapseConfig_t(uint32_t processing_queue_length_) : in_progress_(false), image_input_(processing_queue_length_), frames_processed_(0), frames_written_(0)
		{
			trigger_new_video = false;
		}
		ion::Queue<std::shared_ptr<ion::Image>> image_input_; //the images to process
		std::vector<ion::Queue<EventBase>*> event_output_; //The thread pushes the event output into each queue in this vector
		std::vector<ion::Queue<std::shared_ptr<ion::Image>>*> image_output_; //the thread pushes each new image onto each queue in this vector

		float seconds_per_frame_;
		uint32_t frames_per_video_;

		//debug/instrumentation
		bool in_progress_;
		uint64_t frames_processed_;
		uint64_t frames_written_;
		bool trigger_new_video;
	};

	void timelapseThread(void* args);

};
#endif //SENTRY_TIMELAPSE_H_