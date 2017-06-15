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
#ifndef SENTRY_BACKDOOR_H_
#define SENTRY_BACKDOOR_H_
#include "sentry\CameraIO.h"
#include "sentry\MotionDetection.h"
#include "sentry\Output.h"
#include "sentry\Timelapse.h"
#include "ionlib\backdoor.h"
namespace ion
{
	typedef struct sentry_s
	{
		sentry_s()
		{
			shutdownInProgress = false;
		}
		ion::CameraIoConfig_t* ioConfig;
		ion::MotionDetectionConfig_t* motionDetectionConfig;
		ion::TimelapseConfig_t* timelapseConfig_;
		ion::MotionDetectionEventManagerConfig_t* motionDetectionEventProc;
		ion::CameraOutputConfig_t* motionDetectionOutputConfig;
		ion::CameraOutputConfig_t* timelapseOutputConfig;
		bool motionDetectionEnabled;
		bool shutdownInProgress;
	} sentry_t;

	void init_sentry_backdoor(ion::sentry_t* sentry, ion::Backdoor* backdoor, ion::CameraIoConfig_t * ioConfig, ion::MotionDetectionConfig_t * motionDetectionConfig, ion::MotionDetectionEventManagerConfig_t * motionDetectionEventProc, ion::CameraOutputConfig_t * motionDetectionOutputConfig, bool motionDetectionEnabled, ion::TimelapseConfig_t* timelapseConfig_, ion::CameraOutputConfig_t * timelapseOutputConfig);
	void sentry_stat(ion::Backdoor* bd, std::string args, void* usr_data);
};
#endif //SENTRY_BACKDOOR_H_