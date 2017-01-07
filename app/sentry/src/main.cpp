
#include <opencv2\core.hpp>
#include "ionlib\matrix_opencv.h"
#include "ionlib\log.h"
#include "ionlib\net.h"
#include "ionlib\thread.h"

#include "ffwrapper\ffreader.h"


struct CameraIoConfig_t
{
	uint32_t dummy;
};

void cameraIoThread(void* args)
{
	ion::FFReader reader("C:\\video.mp4");
	while (true)
	{
		ion::Matrix<uint8_t> temp = reader.ReadFrame();
		temp.imshow();
	}
}



int main(int argc, char* argv[])
{
	ion::InitSockets();
	ion::LogInit("sentry");

	//configuration
	CameraIoConfig_t config;
	//launch threads to connect to cameras
		//cameras capture from source and place the image into a queue
	ion::StartThread(cameraIoThread, (void*)&config);

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