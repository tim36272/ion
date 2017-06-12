#include "ionlib/iondef.h"
#include <opencv2\core.hpp>
#include <opencv2\highgui.hpp>
#include "ionlib/matrix_opencv.h"
#include "ionlib/log.h"
#include "ionlib/net.h"
#include "ionlib/thread.h"
#include "ionlib/queue.h"
#include "ionlib/timer.h"
#include "ionlib/config.h"
#include "ionlib/backdoor.h"
#include "ionlib/ascii.h"
#include "ionlib/counter.h"


int main(int argc, char* argv[])
{
	ion::InitSockets();
	ion::LogInit("dominoCounter.log");

	ion::Config cfg("../cfg/dominoCounter.cfg");
	//open domino picture
	cv::Mat orig_dominos = cv::imread(cfg.Getc("SAMPLE_IMAGE"));
	cv::Mat edges;
	cv::Mat bgr[3];
	cv::split(orig_dominos, bgr);
	cv::imshow("OrigBlue", bgr[0]);
	cv::imshow("OrigGreen", bgr[1]);
	cv::imshow("OrigRed", bgr[2]);
	cv::Mat gray_dominos;
	cv::GaussianBlur(bgr[0], bgr[0], cv::Size(3, 3), 2, 2);
	cv::GaussianBlur(bgr[1], bgr[1], cv::Size(3, 3), 2, 2);
	cv::GaussianBlur(bgr[2], bgr[2], cv::Size(3, 3), 2, 2);
	cv::threshold(bgr[0], bgr[0], 75.0, 255, cv::THRESH_BINARY);
	cv::threshold(bgr[1], bgr[1], 150.0, 255, cv::THRESH_BINARY);
	cv::threshold(bgr[2], bgr[2], 150.0, 255, cv::THRESH_BINARY);
	gray_dominos = bgr[0] * 0.5 + bgr[1] * 0.5;// + bgr[2] * 0.33;
	cv::threshold(gray_dominos, gray_dominos, 250, 255, cv::THRESH_BINARY);
	std::vector<cv::Vec3f> circles;
	cv::Canny(gray_dominos, edges, MAX(200 / 2, 1), 100, 3);
	cv::imshow("Canny", edges);
	cv::imshow("Blue", bgr[0]);
	cv::imshow("Green", bgr[1]);
	cv::imshow("Red", bgr[2]);
	cv::imshow("Gray", gray_dominos);
	cv::waitKey(1);
	cv::HoughCircles(gray_dominos, circles, CV_HOUGH_GRADIENT, 1.0, 20, 200, 8, 8, 16);
	for (size_t circle_index = 0; circle_index < circles.size(); ++circle_index)
	{
		cv::Point center(circles[circle_index][0], circles[circle_index][1]);
		int radius = circles[circle_index][2];
		cv::circle(orig_dominos, center, radius, cv::Scalar(255), -1);
	}
	LOGINFO("Found %d circles", circles.size());
	cv::imshow("Test", orig_dominos);
	cv::waitKey(0);
	return 0;
}