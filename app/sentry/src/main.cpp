
#include <opencv2\core.hpp>
#include <opencv2\video.hpp>
#include "ionlib\matrix_opencv.h"
#include "ionlib\log.h"
#include "ionlib\net.h"
#include "ionlib\thread.h"
#include "ionlib\timer.h"
#include "ionlib\config.h"
#include "ionlib\backdoor.h"

#include "ffwrapper\read.h"
#include "ffwrapper\write.h"



struct CameraIoConfig_t
{
	CameraIoConfig_t(std::string camera_uri) : reader(camera_uri)
	{
	}
	ion::FFReader reader;
	ion::Timer fps;
	std::vector<ion::Queue<ion::Image>*> output;
};

class EventBase
{
public:
	enum class Type
	{
		EVENT_STATUS,
		CAMERA_MOTION,
		PIR
	};
	Type event_type_;
	bool in_progress_;
};

struct CameraProcConfig_t
{
	ion::Queue<ion::Matrix<uint8_t>> input_;
	std::vector<ion::Queue<ion::Image>*> image_output_;
	std::vector<ion::Queue<EventBase*>*> event_output_;

	ion::Timer fps_;
	float motion_threshold_;
};

struct CameraEventManagerConfig_t
{
	ion::Queue<EventBase*> input_;
	std::vector<ion::Queue<EventBase>*> output_;
	uint32_t event_spacing_; //time, in milliseconds, between events before considering them finished
};

struct CameraOutputConfig_t
{
	ion::Queue<ion::Image> input_;
	ion::Queue<EventBase> event_;
	ion::Timer fps_;
	ion::FFWriter* writer;
};


void cameraIoThread(void* args)
{
	CameraIoConfig_t* io_proc = (CameraIoConfig_t*)args;
	while (true)
	{
		ion::Image temp = io_proc->reader.ReadFrame();

		io_proc->fps.PeriodBegin();
		LOGINFO("Real FPS: %llf, FFMPEG Fps: %llf", io_proc->fps.GetMean() * 1000, io_proc->reader.GetFps());
		for (std::vector<ion::Queue<ion::Image>*>::iterator it = io_proc->output.begin(); it != io_proc->output.end(); ++it)
		{
			(*it)->Push(temp.DeepCopy());
		}
	}
}

void cameraProcThread(void* args)
{
	CameraProcConfig_t* camera_proc = (CameraProcConfig_t*)args;
	cv::Ptr<cv::BackgroundSubtractorMOG2> mog2 = cv::createBackgroundSubtractorMOG2(500, 16.0, true);
	cv::Mat foreground_mask;

	while (true)
	{
		ion::Matrix<uint8_t> input(0);
		ion::Error result = camera_proc->input_.Pop(0, &input);
		camera_proc->fps_.PeriodBegin();
		if (result.success())
		{
			cv::Mat img = input.asCvMat();
			cv::imshow("raw", img);
			cv::waitKey(1);
			cv::GaussianBlur(img, img, cv::Size(3, 3), 0);
			mog2->apply(img, foreground_mask);
			cv::threshold(foreground_mask, foreground_mask, 255 / 2 + 1, 230, cv::THRESH_BINARY);
			cv::dilate(foreground_mask, foreground_mask, cv::Mat(), cv::Point(-1, -1), 2);
			cv::erode(foreground_mask, foreground_mask, cv::Mat(), cv::Point(-1, -1), 4);
			cv::Mat black(img.rows, img.cols, CV_8UC3, cv::Scalar(0));
			cv::bitwise_or(black, img, black, foreground_mask);
			//LOGINFO("Processing FPS: %lf", 1.0 / (camera_proc->fps_.GetMean() / 1000.0));
			
			//compute the number of moving pixels in the image to detect if this event is in progress
			cv::Scalar num_moving_pixels = cv::sum(foreground_mask);
			EventBase *camera_event = new EventBase;
			camera_event->event_type_ = EventBase::Type::CAMERA_MOTION;
			if ((float)num_moving_pixels[0] / (foreground_mask.rows * foreground_mask.cols) > camera_proc->motion_threshold_)
			{
				camera_event->in_progress_ = true;
			} else
			{
				camera_event->in_progress_ = false;
			}
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
void cameraEventManagerThread(void* args)
{
	CameraEventManagerConfig_t* event_proc = (CameraEventManagerConfig_t*)args;
	EventBase default_event;
	EventBase* input_event;
	EventBase output_event;
	output_event.event_type_ = EventBase::Type::EVENT_STATUS;
	default_event.event_type_ = EventBase::Type::CAMERA_MOTION;
	default_event.in_progress_ = false;
	double last_in_progress_tov = -DBL_MAX;
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
			} else if (ion::TimeGet() - last_in_progress_tov < 5.0) {
				output_event.in_progress_ = true;
			} else
			{
				output_event.in_progress_ = false;
			}
		} else
		{
			//a different error occurred
			LOGINFO("Got result \"%s\" in event manager thread when trying to dequeue a frame", result.str());
			continue;
		}
		//send the event status
		for (std::vector<ion::Queue<EventBase>*>::iterator it = event_proc->output_.begin(); it != event_proc->output_.end(); ++it)
		{
			(*it)->Push(output_event);
		}
	}
}

void cameraOutputThread(void* args)
{
	CameraOutputConfig_t* output_proc = (CameraOutputConfig_t*)args;
	EventBase event_in_progress;
	event_in_progress.in_progress_ = false;
	ion::Error result;
	bool video_file_open = false;
	while (true)
	{
		//the queue automatically ringbuffers, and is sized per the pre-record length. So, when not currently outputting, we only have to listen for events, not frames
		if (event_in_progress.in_progress_ == false)
		{
			result = output_proc->event_.Pop(0, &event_in_progress);
		} else
		{
			result = output_proc->event_.Pop(1, &event_in_progress);
		}
		if (!result.success() && result != ion::Error::Get(ion::Error::TIMEOUT))
		{
			LOGINFO("Got result \"%s\" in the output thread when trying to dequeue an event", result.str());
		}
		LOGASSERT(event_in_progress.event_type_ == EventBase::Type::EVENT_STATUS);
		if (event_in_progress.in_progress_ == true)
		{
			if (!video_file_open)
			{
				//get one frame so we know the dimensions
				ion::Image frame(0);
				output_proc->input_.Pop(0, &frame);
				//open a new video file
				char video_filename[256];
				sprintf_s(video_filename, "output_%lf.mp4", ion::TimeGetEpoch());
				LOGINFO("Opening video file %s", video_filename);
				video_file_open = true;
				output_proc->writer = new ion::FFWriter(video_filename, frame.rows(), frame.cols());
				output_proc->writer->WriteFrame(frame);
			}
			//dequeue all of the frames currently in the buffer
			size_t frames_to_dequeue = output_proc->input_.size();
			if (frames_to_dequeue > 0)
			{
				LOGINFO("Dequing %llu frames", frames_to_dequeue);
				ion::Image frame(0);
				for (size_t frame_index = 0; frame_index < frames_to_dequeue; ++frame_index)
				{
					//write frames to file
					output_proc->input_.Pop(0, &frame);
					output_proc->writer->WriteFrame(frame);
				}
			} else
			{
				ion::ThreadSleep(0);
			}
		} else
		{
			//close the video file
			//TODO
			if (video_file_open)
			{
				video_file_open = false;
				LOGINFO("Closing video file");
				output_proc->writer->Close();
				delete output_proc->writer;
			}
		}
	}
}



int main(int argc, char* argv[])
{
	ion::InitSockets();
	ion::LogInit("sentry.log");

	ion::Config cfg("sentry.cfg");
	cfg.AddFile("credentials.cfg");

	//configuration
	char uri[256];
	//sprintf_s(uri, "http://ipcam3.elements:81/videostream.cgi?loginuse=%s&loginpas=%s", cfg.Getc("CAMERA_USERNAME"), cfg.Getc("CAMERA_PASSWORD"));
	sprintf_s(uri, "rtsp://%s:%s@192.168.16.16:554/udp/av0_0", cfg.Getc("CAMERA_USERNAME"), cfg.Getc("CAMERA_PASSWORD"));
	ion::IpPort backdoor_port(cfg.Gets("BACKDOOR_PORT"), ion::IpPort::Order::HOST);
	ion::Backdoor backdoor(backdoor_port);
	CameraIoConfig_t ioConfig(uri);
	CameraProcConfig_t procConfig;
	CameraEventManagerConfig_t eventProc;
	CameraOutputConfig_t outputConfig;

	ioConfig.output.push_back(&procConfig.input_);
	ioConfig.output.push_back(&outputConfig.input_);
	procConfig.event_output_.push_back(&eventProc.input_);
	procConfig.motion_threshold_ = cfg.Getf("MOTION_THRESHOLD");
	eventProc.output_.push_back(&outputConfig.event_);

	//launch threads to connect to cameras
		//cameras capture from source and place the image into a queue
	ion::StartThread(cameraIoThread, (void*)&ioConfig);
	ion::StartThread(cameraProcThread, (void*)&procConfig);
	ion::StartThread(cameraEventManagerThread, (void*)&eventProc);
	ion::StartThread(cameraOutputThread, (void*)&outputConfig);

	ion::SuspendCurrentThread();
	//launch threads to process images
		//one thread per camera waits for the output one-to-many double buffer, then waits on the camera's queue, dequeues an image, processes it into shared memory, then posts the image to the one-to-many double buffer
	//launch threads to output images
		//one thread per destination (i.e. many per camera) wait on a one-to-many double buffer, output the image, then return it to the one-to-many double buffer

	//one-to-many double buffer logic:
		//initially, all buffers are ready for the producer. The producer gets a buffer by getting N semaphores from the producer semaphore pool.
		//	It produces the buffer by posting N semaphores to that buffer's consumer semaphore pool. The consumers are waiting on those semaphores.
		//	They get the data, process it, and when complete they post one semaphore each to the producer semaphore pool
	return 0;
}