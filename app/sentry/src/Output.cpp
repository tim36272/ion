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
#include <sentry\Output.h>
#include <sentry\Common.h>
namespace ion
{
	void cameraOutputThread(void* args)
	{
		CameraOutputConfig_t* output_proc = (CameraOutputConfig_t*)args;
		EventBase event_in_progress;
		event_in_progress.in_progress_ = false;
		ion::Error result;
		while (true)
		{
			//the queue automatically ringbuffers, and is sized per the pre-record length. So, when not currently outputting, we only have to listen for events, not frames
			if (event_in_progress.in_progress_ == false)
			{
				result = output_proc->event_input_.Pop(QUEUE_WAIT_FOREVER, &event_in_progress);
			} else
			{
				uint32_t ms_to_wait = 1;
				result = output_proc->event_input_.Pop(ms_to_wait, &event_in_progress);
			}
			if (!result.success() && result != ion::Error::Get(ion::Error::TIMEOUT))
			{
				LOGINFO("Got result \"%s\" in the output thread when trying to dequeue an event", result.str());
			}
			LOGASSERT(event_in_progress.event_type_ == EventBase::Type::EVENT_STATUS);
			if (event_in_progress.in_progress_ == true)
			{
				if (!output_proc->video_file_open_)
				{
					//get one frame so we know the dimensions
					std::shared_ptr<ion::Image> frame;
					output_proc->image_input_.Pop(0, &frame);
					//open a new video file
					char video_filename[256];
					sprintf_s(video_filename, "%s\\%s_%lf.mp4", output_proc->output_dir_.c_str(), output_proc->camera_name_.c_str(), ion::TimeGetEpoch());
					LOGINFO("Opening video file %s", video_filename);
					output_proc->video_file_open_ = true;
					output_proc->writer = new ion::FFWriter(video_filename, frame->rows(), frame->cols());
					output_proc->writer->WriteFrame(*frame);
					output_proc->fps_.Reset();
				}
				//dequeue all of the frames currently in the buffer
				size_t frames_to_dequeue = output_proc->image_input_.size();
				if (frames_to_dequeue > 0)
				{
					std::shared_ptr<ion::Image> frame(0);
					for (size_t frame_index = 0; frame_index < frames_to_dequeue; ++frame_index)
					{
						//write frames to file
						output_proc->image_input_.Pop(0, &frame);
						output_proc->writer->WriteFrame(*frame);
						output_proc->fps_.PeriodBegin();
					}
				} else
				{
					//We are spinning faster than the camera, so sleep for a bit to prevent this thread from busy waiting
					ion::ThreadSleep(1000);
				}
			} else
			{
				//close the video file
				if (output_proc->video_file_open_)
				{
					output_proc->video_file_open_ = false;
					LOGINFO("Closing video file");
					output_proc->writer->Close();
					delete output_proc->writer;
				}
			}
		}
	}
};