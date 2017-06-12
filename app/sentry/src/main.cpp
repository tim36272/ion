#include "ionlib/config.h"

//#define SENTRY_DEBUG
#include "sentry/CameraIO.h"
#include "sentry/Common.h"
#include "sentry/MotionDetection.h"
#include "sentry/Timelapse.h"
#include "sentry/Output.h"
#include "sentry/Backdoor.h"

int main(int argc, char* argv[])
{
	ion::InitSockets();
	ion::LogInit("sentry.log");

	ion::Config cfg("sentry.cfg");
	for (uint32_t additional_config_index = 1; additional_config_index < (uint32_t)argc; ++additional_config_index)
	{
		cfg.AddFile(argv[additional_config_index]); //credentials.cfg, camera.cfg
	}

	//configuration
	char uri[256];
	//sprintf_s(uri, "http://ipcam3.elements:81/videostream.cgi?loginuse=%s&loginpas=%s", cfg.Getc("CAMERA_USERNAME"), cfg.Getc("CAMERA_PASSWORD"));
	sprintf_s(uri, "rtsp://%s:%s@%s:554/udp/av0_0", cfg.Getc("CAMERA_USERNAME"), cfg.Getc("CAMERA_PASSWORD"), cfg.Getc("CAMERA_IP"));

	//Create datastructures
	bool motion_detection_enabled = cfg.Getbool("MOTION_DETECTION_ENABLED");

	ion::IpPort backdoor_port(cfg.Gets("BACKDOOR_PORT"), ion::IpPort::Order::HOST);
	ion::CameraIoConfig_t ioConfig(uri);
	ion::TimelapseConfig_t timelapseConfig(cfg.Getu("PROCESSING_QUEUE_LENGTH"));
	ion::CameraOutputConfig_t timelapseOutputConfig(cfg.Getu("PRERECORD_FRAMES"));

	ion::MotionDetectionConfig_t* motionDetection_p = nullptr;
	ion::MotionDetectionEventManagerConfig_t* motionDetectionEventProc_p = nullptr;
	ion::CameraOutputConfig_t* motionDetectionOutputConfig_p = nullptr;
	if(motion_detection_enabled)
	{
		motionDetection_p = new ion::MotionDetectionConfig_t(cfg.Getu("PROCESSING_QUEUE_LENGTH"));
		motionDetectionEventProc_p = new ion::MotionDetectionEventManagerConfig_t;
		motionDetectionOutputConfig_p = new ion::CameraOutputConfig_t(cfg.Getu("PRERECORD_FRAMES"));

		ioConfig.output.push_back(&motionDetection_p->input_);						//Queue for motion detection processing
		ioConfig.output.push_back(&motionDetectionOutputConfig_p->image_input_);	//Queue for motion detection output

																					//Configure Motion detection
		motionDetection_p->event_output_.push_back(&motionDetectionEventProc_p->input_);
		motionDetection_p->motion_threshold_ = cfg.Getf("MOTION_THRESHOLD");
		motionDetection_p->lightswitch_suppression_threshold_ = cfg.Getf("LIGHTSWITCH_SUPRESSION_THRESHOLD");
		motionDetection_p->warmup_frames_ = cfg.Getu("MOTION_WARMUP_FRAMES");

		//Configure event management
		motionDetectionEventProc_p->output_.push_back(&motionDetectionOutputConfig_p->event_input_);
		motionDetectionEventProc_p->motion_backoff_ = cfg.Getf("MOTION_BACKOFF");
		motionDetectionEventProc_p->post_record_time_ = cfg.Getf("POSTRECORD_TIME");
		motionDetectionOutputConfig_p->output_dir_ = cfg.Getc("OUTPUT_DIR");
		motionDetectionOutputConfig_p->camera_name_ = cfg.Getc("CAMERA_NAME");


		//launch threads to process images
		ion::StartThread(ion::motionDetectionThread, (void*)motionDetection_p);
		ion::StartThread(ion::motionDetectionEventManagerThread, (void*)motionDetectionEventProc_p);

		//launch threads to output images
		ion::StartThread(ion::cameraOutputThread, (void*)motionDetectionOutputConfig_p);
	}
	
	//Setup backdoor
	ion::Backdoor backdoor(backdoor_port);
	ion::init_sentry_backdoor(&backdoor, &ioConfig, motionDetection_p, motionDetectionEventProc_p, motionDetectionOutputConfig_p, motion_detection_enabled, &timelapseConfig, &timelapseOutputConfig);

	//Configure IO

	ioConfig.output.push_back(&timelapseConfig.image_input_);				//Queue for timelapse input (it forwards its own input)

	//Configure Timelapse
	timelapseConfig.event_output_.push_back(&timelapseOutputConfig.event_input_);
	timelapseConfig.frames_per_video_ = cfg.Getu("TIMELAPSE_FRAMES_PER_VIDEO");
	timelapseConfig.seconds_per_frame_ = cfg.Getf("TIMELAPSE_SECONDS_PER_FRAME");
	timelapseConfig.image_output_.push_back(&timelapseOutputConfig.image_input_);
	timelapseOutputConfig.output_dir_ = cfg.Getc("OUTPUT_DIR");
	timelapseOutputConfig.camera_name_ = cfg.Getc("CAMERA_NAME");


	//launch threads to connect to cameras
	//cameras capture from source and place the image into a queue
	ion::StartThread(ion::cameraIoThread, (void*)&ioConfig);


	//launch threads to process images
	ion::StartThread(ion::timelapseThread, (void*)&timelapseConfig);

	//launch threads to output images
	ion::StartThread(ion::cameraOutputThread, (void*)&timelapseOutputConfig);


	ion::SuspendCurrentThread();
	return 0;
}