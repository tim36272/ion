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
#include <opencv2\imgproc.hpp>
#include "ionlib\matrix_opencv.h"
#include "ionlib\timer.h"
#include "sentry\Timelapse.h"
namespace ion
{
	void timelapseThread(void* args)
	{
		TimelapseConfig_t* timelapse_config = (TimelapseConfig_t*)args;
		EventBase output_event;
		output_event.event_type_ = EventBase::Type::EVENT_STATUS;
		output_event.in_progress_ = true;
		ion::Timer timelapse_timer;

		//This is the main loop
		while (true)
		{
			//Toggle the event status so the output thread will start a new video
			for (std::vector<ion::Queue<EventBase>*>::iterator it = timelapse_config->event_output_.begin(); it != timelapse_config->event_output_.end(); ++it)
			{
				output_event.in_progress_ = false;
				(*it)->Push(output_event);
				output_event.in_progress_ = true;
				(*it)->Push(output_event);
			}
			timelapse_timer.Begin();
			bool trigger_new_video = false;
			uint32_t frames_written_this_event = 0;
			for (uint32_t this_frame_index = 0; frames_written_this_event < timelapse_config->frames_per_video_ && !trigger_new_video; ++this_frame_index)
			{
				std::shared_ptr<ion::Image> input;
				ion::Error result = timelapse_config->image_input_.Pop(0, &input);
				if (result.success())
				{
					timelapse_config->frames_processed_++;
				} else
				{
					LOGINFO("Got result \"%s\" in timelapse thread when trying to dequeue a frame", result.str());
				}
				double time_since_last_output = timelapse_timer.GetCurrent();
				if (time_since_last_output > timelapse_config->seconds_per_frame_)
				{
					timelapse_timer.End();
					timelapse_timer.Begin();
					//this frame should be output. Put the datetime on it
					std::shared_ptr<ion::Image> temp_img(new ion::Image(*input));
					cv::Mat input_mat = temp_img->asCvMat();
					char time_buf[32];
					sprintf_s(time_buf, "%.3lf", ion::TimeGetEpoch());
					cv::putText(input_mat, time_buf, cv::Point(10, 40), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 2);
					for (std::vector<ion::Queue<std::shared_ptr<ion::Image>>*>::iterator it = timelapse_config->image_output_.begin(); it != timelapse_config->image_output_.end(); ++it)
					{
						(*it)->Push(temp_img);
					}
					++timelapse_config->frames_written_;
					++frames_written_this_event;
				}
				trigger_new_video = timelapse_config->trigger_new_video;
			}
			if (trigger_new_video)
			{
				timelapse_config->trigger_new_video = false;
			}


		}

	}
};