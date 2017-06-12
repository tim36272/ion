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
#ifndef SENTRY_MOTION_DETECTION_
#define SENTRY_MOTION_DETECTION_
#include "sentry\Common.h"
#include "ionlib\iondef.h"
#include "ionlib\queue.h"
#include "ionlib\timer.h"
#include "ionlib\counter.h"
#include "ffwrapper\common.h"
namespace ion
{
	struct MotionDetectionConfig_t
	{
		MotionDetectionConfig_t(uint32_t processing_queue_length_) : in_progress_(false), input_(processing_queue_length_), frames_processed_(0)
		{
		}
		ion::Queue<std::shared_ptr<ion::Image>> input_; //the images to process
		std::vector<ion::Queue<EventBase*>*> event_output_; //The thread pushes the event output into each queue in this vector

		float motion_threshold_;
		float lightswitch_suppression_threshold_;
		uint32_t warmup_frames_;

		//debug/instrumentation
		ion::Timer fps_;
		ion::Counter<uint32_t> pixel_change_counter_;
		ion::Counter<double> percent_change_counter_;
		bool in_progress_;
		uint64_t frames_processed_;
	};

	struct MotionDetectionEventManagerConfig_t
	{
		MotionDetectionEventManagerConfig_t() : in_progress_(false)
		{
		}
		ion::Queue<EventBase*> input_;
		std::vector<ion::Queue<EventBase>*> output_; //the thread pushes the event output into each queue in this vector
		uint32_t event_spacing_; //time, in milliseconds, between events before considering them finished
		double   motion_backoff_; //time, in seconds, between when event is first detected and when it is activated
		double   post_record_time_; //time, in seconds, after an event ends to continue recording
									//debug/instrumentation
		bool in_progress_;
	};

	void motionDetectionThread(void* args);
	void motionDetectionEventManagerThread(void* args);
};
#endif //SENTRY_MOTION_DETECTION_