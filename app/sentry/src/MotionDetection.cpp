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
#include <opencv2\video.hpp>
#include <opencv2\highgui.hpp>

#include "sentry\MotionDetection.h"
namespace ion
{
	void motionDetectionThread(void* args)
	{
		MotionDetectionConfig_t* camera_proc = (MotionDetectionConfig_t*)args;
		cv::Ptr<cv::BackgroundSubtractorMOG2> mog2 = cv::createBackgroundSubtractorMOG2(500, 16.0, false);
		cv::Mat foreground_mask;

		//process warmup frames
		for (uint32_t frame_index = 0; frame_index < camera_proc->warmup_frames_; ++frame_index)
		{
			std::shared_ptr<ion::Image> input;
			ion::Error result = camera_proc->input_.Pop(0, &input);
			if (result.success())
			{
				camera_proc->frames_processed_++;
				cv::Mat img = input->asCvMat();
				cv::GaussianBlur(img, img, cv::Size(3, 3), 0);
				mog2->apply(img, foreground_mask);
			}
		}

		//This is the main loop
		while (true)
		{
			std::shared_ptr<ion::Image> input;
			ion::Error result = camera_proc->input_.Pop(0, &input);
			camera_proc->fps_.PeriodBegin();
			if (result.success())
			{
				camera_proc->frames_processed_++;
				cv::Mat img = input->asCvMat();
				cv::GaussianBlur(img, img, cv::Size(3, 3), 0);
				mog2->apply(img, foreground_mask);
				cv::threshold(foreground_mask, foreground_mask, 255 / 2 + 1, 230, cv::THRESH_BINARY);
				cv::dilate(foreground_mask, foreground_mask, cv::Mat(), cv::Point(-1, -1), 2);
				cv::erode(foreground_mask, foreground_mask, cv::Mat(), cv::Point(-1, -1), 6);
				cv::Mat black(img.rows, img.cols, CV_8UC3, cv::Scalar(0));
				cv::bitwise_or(black, img, black, foreground_mask);
				cv::imshow("processed", foreground_mask);
				cv::waitKey(1);

				//compute the number of moving pixels in the image to detect if this event is in progress
				cv::Scalar num_moving_pixels = cv::sum(foreground_mask) / 256.0;
				camera_proc->pixel_change_counter_.Add((uint32_t)num_moving_pixels[0]);
				EventBase *camera_event = new EventBase;
				camera_event->event_type_ = EventBase::Type::CAMERA_MOTION;
				double percent_change = (double)num_moving_pixels[0] / (foreground_mask.rows * foreground_mask.cols);
				camera_proc->percent_change_counter_.Add(percent_change);
				if (percent_change > camera_proc->motion_threshold_ && percent_change < camera_proc->lightswitch_suppression_threshold_)
				{
					camera_event->in_progress_ = true;

				} else
				{
					camera_event->in_progress_ = false;
				}
				camera_proc->in_progress_ = camera_event->in_progress_;
				for (std::vector<ion::Queue<EventBase*>*>::iterator it = camera_proc->event_output_.begin(); it != camera_proc->event_output_.end(); ++it)
				{
					(*it)->Push(camera_event);
				}
			} else
			{
				LOGINFO("Got result \"%s\" in processing thread when trying to dequeue a frame", result.str());
			}
		}

	}

	void motionDetectionEventManagerThread(void* args)
	{
		MotionDetectionEventManagerConfig_t* event_proc = (MotionDetectionEventManagerConfig_t*)args;
		EventBase default_event;
		EventBase* input_event;
		EventBase output_event;
		output_event.event_type_ = EventBase::Type::EVENT_STATUS;
		default_event.event_type_ = EventBase::Type::CAMERA_MOTION;
		default_event.in_progress_ = false;
		double last_in_progress_tov = -DBL_MAX;
		bool   event_started = false;
		double event_start_tov = -DBL_MAX;
		while (true)
		{
			//wait for events
			ion::Error result = event_proc->input_.Pop(event_proc->event_spacing_, &input_event);
			//treat timeouts as a non-event
			if (result == ion::Error::Get(ion::Error::TIMEOUT))
			{
				input_event = &default_event;
				result = ion::Error::Get(ion::Error::SUCCESS);
			}
			if (result.success())
			{
				if (input_event->event_type_ == EventBase::Type::CAMERA_MOTION && input_event->in_progress_ == true)
				{
					//tell the output thread to record
					output_event.in_progress_ = true;
					last_in_progress_tov = ion::TimeGet();
				} else if (ion::TimeGet() - last_in_progress_tov < 5.0)
				{
					output_event.in_progress_ = true;
				} else
				{
					output_event.in_progress_ = false;
					event_started = false;
				}
			} else
			{
				//a different error occurred
				LOGINFO("Got result \"%s\" in event manager thread when trying to dequeue a frame", result.str());
				continue;
			}
			event_proc->in_progress_ = output_event.in_progress_;
			//Check if the event has been ongoing long enough to be considered active
			if (event_proc->in_progress_)
			{
				if (!event_started)
				{
					event_started = true;
					event_start_tov = ion::TimeGet();
				}
				if (ion::TimeGet() - event_start_tov < event_proc->motion_backoff_)
				{
					//don't tell the outputter that the event is running yet
					output_event.in_progress_ = false;
				}
			}


			//send the event status
			for (std::vector<ion::Queue<EventBase>*>::iterator it = event_proc->output_.begin(); it != event_proc->output_.end(); ++it)
			{
				(*it)->Push(output_event);
			}
		}
	}
};