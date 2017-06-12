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
#include <opencv2\highgui.hpp>
#include "sentry\CameraIO.h"
#include "ionlib\matrix_opencv.h"
namespace ion
{
	void cameraIoThread(void* args)
	{
		CameraIoConfig_t* io_proc = (CameraIoConfig_t*)args;
		while (true)
		{
			std::shared_ptr<ion::Image> temp_img(new ion::Image(std::move(io_proc->reader.ReadFrame())));

			io_proc->fps_.PeriodBegin();
			//LOGINFO("Real FPS: %llf, FFMPEG Fps: %llf", io_proc->fps_.GetMean() * 1000, io_proc->reader.GetFps());
			for (std::vector<ion::Queue<std::shared_ptr<ion::Image>>*>::iterator it = io_proc->output.begin(); it != io_proc->output.end(); ++it)
			{
				(*it)->Push(temp_img);
			}
#ifdef SENTRY_DEBUG
			cv::imshow("raw", temp_img->asCvMat());
			cv::waitKey(1);
#endif
		}
	}
};