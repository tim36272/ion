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
#include "sentry\Backdoor.h"
#include "ionlib\ascii.h"
namespace ion
{
	static void sentry_shutdown(ion::Backdoor* backdoor, std::string args, void* usr_data);
	void init_sentry_backdoor(ion::sentry_t* sentry, ion::Backdoor* backdoor, ion::CameraIoConfig_t * ioConfig, ion::MotionDetectionConfig_t * motionDetectionConfig, ion::MotionDetectionEventManagerConfig_t * motionDetectionEventProc, ion::CameraOutputConfig_t * motionDetectionOutputConfig, bool motionDetectionEnabled, ion::TimelapseConfig_t* timelapseConfig_, ion::CameraOutputConfig_t * timelapseOutputConfig)
	{
		sentry->ioConfig = ioConfig;
		sentry->motionDetectionConfig = motionDetectionConfig;
		sentry->motionDetectionEventProc = motionDetectionEventProc;
		sentry->motionDetectionOutputConfig = motionDetectionOutputConfig;
		sentry->motionDetectionEnabled = motionDetectionEnabled;
		sentry->timelapseConfig_ = timelapseConfig_;
		sentry->timelapseOutputConfig = timelapseOutputConfig;
		backdoor->AddCommand("stat", "Summarize sentry status", ion::sentry_stat, sentry);
		backdoor->AddCommand("shutdown", "Safely close all streams", ion::sentry_shutdown, sentry);
		ion::LogAddBackdoor(backdoor);
	}
	static void sentry_shutdown(ion::Backdoor* backdoor, std::string args, void* usr_data)
	{
		sentry_t* sentry = (sentry_t*)usr_data;
		sentry->ioConfig->shutdownInProgress = true;
		sentry->timelapseOutputConfig->shutdownInProgress = true;
		if (sentry->motionDetectionEnabled)
		{
			sentry->motionDetectionOutputConfig->shutdownInProgress = true;
		}
		sentry->shutdownInProgress = true;
	}
	//This function throughly violates my normal rules of descriptive variable names because the lines are already so long
	void sentry_stat(ion::Backdoor* bd, std::string args, void* usr_data)
	{
		sentry_t* s = (sentry_t*)usr_data;
		LOGASSERT(s != nullptr);
		char selected_command = '-';
		//clear the screen
		bd->ClearScreen();
		//Clear the input buffer
		bd->ClearInputBuffer();
		for (;;)
		{
			uint32_t r = 1;
			uint32_t c = 0;

			char input = bd->GetInput();
			if (input == ion::CHAR_ESCAPE)
			{
				return;
			}
			bd->SetCursor(r++, c);                           bd->printf("+------------------------------""------------------------------""------------------------"      "+");
			bd->SetCursor(r++, c); bd->SetColorHeader();     bd->printf("|Camera IO Thread              ""                              ""                        "      "|");
			bd->SetCursor(r++, c); bd->SetColorNormal();     bd->printf("A  FPS:            %9.1lf "     "| Since last frame: %7.3lf "   "|                           "  "|", 1.0 / s->ioConfig->fps_.GetMean(), ion::TimeGet() - s->ioConfig->fps_.GetStart());
			if (s->motionDetectionEnabled)
			{
				bd->SetCursor(r++, c); bd->SetColorHeader(); bd->printf("|Motion Detection Thread       ""                              ""                        "      "|");
				bd->SetCursor(r++, c); bd->SetColorNormal(); bd->printf("B  FPS:            %9.1lf "     "| Since last frame: %7.3lf "   "C Frames processed:%8llu "     "|", 1.0 / s->motionDetectionConfig->fps_.GetMean(), ion::TimeGet() - s->motionDetectionConfig->fps_.GetStart(), s->motionDetectionConfig->frames_processed_);
				bd->SetCursor(r++, c);                       bd->printf("D  Pix change thresh:  %4.1f%% ""|                           "  "|                           "  "|", s->motionDetectionConfig->motion_threshold_ * 100.0);
				bd->SetCursor(r++, c);                       bd->printf("E  Avg pix changed: %8u "       "| Max pix changed: %8u "       "| Last pix change: %8u "       "|", (uint32_t)s->motionDetectionConfig->pixel_change_counter_.GetMean(), s->motionDetectionConfig->pixel_change_counter_.GetMax(), s->motionDetectionConfig->pixel_change_counter_.GetLast());
				bd->SetCursor(r++, c);                       bd->printf("F  Avg changed:   %9.0lf%% "    "| Max changed:       %5.1lf%% ""| Last change:       %5.1lf%% ""|", s->motionDetectionConfig->percent_change_counter_.GetMean() * 100.0, s->motionDetectionConfig->percent_change_counter_.GetMax() * 100.0, s->motionDetectionConfig->percent_change_counter_.GetLast() * 100.0);
				bd->SetCursor(r++, c);                       bd->printf("|  Image queue len:     %4llu " "|                           "  "                            "  "|", s->motionDetectionConfig->input_.size());
				bd->SetCursor(r++, c); bd->SetColorHeader(); bd->printf("|Motion Detection Event Thread ""                              ""                        "      "|");
				bd->SetCursor(r++, c); bd->SetColorNormal(); bd->printf("|  In progress:        %5s "    "|                           "  "|                           "  "|", s->motionDetectionEventProc->in_progress_ ? "TRUE" : "FALSE");
				bd->SetCursor(r++, c);                       bd->printf("|  Event queue len:     %4llu " "|                           "  "|                           "  "|", s->motionDetectionEventProc->input_.size());
				bd->SetCursor(r++, c); bd->SetColorHeader(); bd->printf("|Motion Detection Output     "  "                            "  "                            "  "|");
				bd->SetCursor(r++, c); bd->SetColorNormal(); bd->printf("G  FPS:            %9.3lf "     "| Since last frame: %7.3lf "   "|                           "  "|", 1.0 / s->motionDetectionOutputConfig->fps_.GetMean(), ion::TimeGet() - s->motionDetectionOutputConfig->fps_.GetStart());
				bd->SetCursor(r++, c);                       bd->printf("|  Video open:         %5s "    "|                           "  "|                           "  "|", s->motionDetectionOutputConfig->video_file_open_ ? "TRUE" : "FALSE");
				bd->SetCursor(r++, c);                       bd->printf("|  Event queue len:     %4llu " "| Image queue len:     %4llu " "| Selected command: %c       " "|", s->motionDetectionOutputConfig->event_input_.size(), s->motionDetectionOutputConfig->image_input_.size(), selected_command);
			}
			bd->SetCursor(r++, c); bd->SetColorHeader();     bd->printf("|Timelapse Thread              ""                              ""                        "      "|");
			bd->SetCursor(r++, c); bd->SetColorNormal();     bd->printf("|  In progress         %5s "    "H Frames processed:%8llu "     "I Trigger new video:  %5s "    "|", s->timelapseConfig_->in_progress_ ? "TRUE" : "FALSE", s->timelapseConfig_->frames_processed_, s->timelapseConfig_->trigger_new_video ? "TRUE" : "FALSE");
			bd->SetCursor(r++, c);                           bd->printf("|  Frames written: %8llu  "     "|                           "  "|                           "  "|", s->timelapseConfig_->frames_written_);
			bd->SetCursor(r++, c); bd->SetColorHeader();     bd->printf("|Timelapse Output            "  "                            "  "                            "  "|");
			bd->SetCursor(r++, c); bd->SetColorNormal();     bd->printf("J  FPS:            %9.3lf "     "| Since last frame: %7.3lf "   "|                           "  "|", 1.0 / s->timelapseOutputConfig->fps_.GetMean(), ion::TimeGet() - s->timelapseOutputConfig->fps_.GetStart());
			bd->SetCursor(r++, c);                           bd->printf("|  Video open:         %5s "    "|                           "  "|                           "  "|", s->timelapseOutputConfig->video_file_open_ ? "TRUE" : "FALSE");
			bd->SetCursor(r++, c);                           bd->printf("|  Event queue len:     %4llu " "| Image queue len:     %4llu " "| Selected command: %c       " "|", s->timelapseOutputConfig->event_input_.size(), s->timelapseOutputConfig->image_input_.size(), selected_command);
			bd->SetCursor(r++, c);                           bd->printf("+------------------------------""------------------------------""------------------------"      "+");

			//put the cursor at the bottom of the screen so that new log messages will show up there
			r++;
			bd->SetCursor(r++, c);

			switch (toupper(input))
			{
				case 'A':
					s->ioConfig->fps_.Reset();
					break;
				case 'B':
					if (s->motionDetectionEnabled)
					{
						s->motionDetectionConfig->fps_.Reset();
					}
					break;
				case 'C':
					if (s->motionDetectionEnabled)
					{
						s->motionDetectionConfig->frames_processed_ = 0;
					}
					break;
				case 'D':
					selected_command = toupper(input);
					break;
				case 'E':
					if (s->motionDetectionEnabled)
					{
						s->motionDetectionConfig->pixel_change_counter_.Reset();
					}
					break;
				case 'F':
					if (s->motionDetectionEnabled)
					{
						s->motionDetectionConfig->percent_change_counter_.Reset();
					}
					break;
				case 'G':
					if (s->motionDetectionEnabled)
					{
						s->motionDetectionOutputConfig->fps_.Reset();
					}
					break;
				case 'H':
					s->timelapseConfig_->frames_processed_ = 0;
					break;
				case 'I':
					s->timelapseConfig_->trigger_new_video = true;
					break;
				case 'J':
					s->timelapseOutputConfig->fps_.Reset();
					break;
				case '-':
					switch (selected_command)
					{
						case '-':
							break;
						case 'D':
							if (s->motionDetectionEnabled)
							{
								s->motionDetectionConfig->motion_threshold_ -= 0.01f;
							}
							break;
						default:
							LOGFATAL("Invalid command %c", selected_command);
							break;
					}
					break;
				case '+':
					switch (selected_command)
					{
						case '-':
							break;
						case 'D':
							if (s->motionDetectionEnabled)
							{
								s->motionDetectionConfig->motion_threshold_ += 0.01f;
							}
							break;
						default:
							LOGFATAL("Invalid command %c", selected_command);
							break;
					}
					break;
				default:
					//do nothing
					break;
			}
			ion::ThreadSleep(100);
		}
	}
};