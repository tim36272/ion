#include "ionlib\log.h"
#include "ionlib\net.h"
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <vector>
#define TRAINING_MODE 0
typedef struct region_s
{
	uint32_t upper_left_x;
	uint32_t upper_left_y;
	uint32_t lower_right_x;
	uint32_t lower_right_y;
} region_t;
typedef struct callbackData_s
{
	region_t regions[73];
	bool regions_dirty;
	uint32_t region_index;
	uint32_t page_splits_at[16];
	uint32_t page_split_index;
} callbackData_t;
void callback(int event, int x, int y, int flags, void* userdata)
{
	static 
	callbackData_t* data = (callbackData_t*)userdata;
	if (event == cv::EVENT_LBUTTONDOWN)
	{
		data->regions[data->region_index].upper_left_x = x;
		data->regions[data->region_index].upper_left_y = y;
	}
	if (event == cv::EVENT_LBUTTONUP)
	{
		data->regions[data->region_index].lower_right_x = x;
		data->regions[data->region_index].lower_right_y = y;
		data->region_index++;
		data->regions_dirty = true;
	}
}
int main(int argc, char* argv[])
{
	if (argc < 4)
	{
		LOGERROR("Usage: app.exe images_file.txt [1: enable training] regions.txt");
	}
	char image_path[512];
	cv::Mat blank_images[16];
	cv::Mat test_images[16];
	ion::InitSockets();
	ion::LogInit("scantron");
	callbackData_t callback_data;
	callback_data.regions_dirty = false;
	callback_data.region_index = 0;
	callback_data.page_split_index = 0;
	memset(callback_data.page_splits_at, 0, sizeof(callback_data.page_splits_at));
	//load the image files
	FILE* file_list = NULL;
	errno_t result = fopen_s(&file_list, argv[1], "r");
	if (!file_list || result != 0)
	{
		LOGFATAL("Failed to load %s", argv[1]);
	}
	//read each line in the file
	for (uint32_t image_index = 0; image_index < 16; ++image_index)
	{
		(void)fscanf_s(file_list, "%s", image_path, 512);
		//open each of the images
		LOGINFO("Opening %s", image_path);
		blank_images[image_index] = cv::imread(image_path, cv::IMREAD_GRAYSCALE);
		cv::resize(blank_images[image_index], blank_images[image_index], cv::Size(blank_images[image_index].cols / 2, blank_images[image_index].rows / 2));
		cv::imshow("test", blank_images[image_index]);
		if (argv[2][0] == '1')
		{
			cv::setMouseCallback("test", callback, &callback_data);
			while (cv::waitKey(1) != ' ')
			{
				if (callback_data.regions_dirty)
				{
					//draw rectangles
					cv::Rect box(callback_data.regions[callback_data.region_index - 1].upper_left_x,
								 callback_data.regions[callback_data.region_index - 1].upper_left_y,
								 callback_data.regions[callback_data.region_index - 1].lower_right_x - callback_data.regions[callback_data.region_index - 1].upper_left_x,
								 callback_data.regions[callback_data.region_index - 1].lower_right_y - callback_data.regions[callback_data.region_index - 1].upper_left_y);
					cv::rectangle(blank_images[image_index], box, cv::Scalar(0));
					cv::imshow("test", blank_images[image_index]);
					callback_data.regions_dirty = false;
				}
			}
			callback_data.page_splits_at[callback_data.page_split_index++] = callback_data.region_index;
		}
	}
	FILE* region_file = NULL;
	if (argv[2][0] == '0')
	{
		//read from file
		result = fopen_s(&region_file, "regions.txt", "rb");
		if (region_file == NULL || result != 0)
		{
			LOGFATAL("Failed to open regions.txt");
		}
		fread(&callback_data, sizeof(callback_data), 1, region_file);
		fclose(region_file);
	}
	//now that the system is trained, dump the region data
	if (argv[2][0] == '1')
	{
		result = fopen_s(&region_file, "regions.txt", "wb");
		if (region_file == NULL || result != 0)
		{
			LOGFATAL("Failed to open regions.txt");
		}
		fwrite(&callback_data, sizeof(callback_data), 1, region_file);
		fclose(region_file);
	}
	//now load the test images and see how well the regions line up
	//read each line in the file
	uint32_t region_index = 0;
	uint32_t page_split_index = 0;
	LOGINFO("Entering grading mode, press space bar to see next page");
	for (uint32_t image_index = 0; image_index < 16; ++image_index)
	{
		(void)fscanf_s(file_list, "%s", image_path, 512);
		//open each of the images
		LOGINFO("Opening %s", image_path);
		test_images[image_index] = cv::imread(image_path, cv::IMREAD_GRAYSCALE);
		cv::resize(test_images[image_index], test_images[image_index], cv::Size(test_images[image_index].cols / 2, test_images[image_index].rows / 2));
		while (region_index < callback_data.page_splits_at[page_split_index])
		{
			uint32_t ulx, uly, lrx, lry;
			ulx = callback_data.regions[region_index].upper_left_x;
			uly = callback_data.regions[region_index].upper_left_y;
			lrx = callback_data.regions[region_index].lower_right_x;
			lry = callback_data.regions[region_index].lower_right_y;
			uint32_t width = lrx - ulx;
			uint32_t height = lry - uly;

			//register the circles
			//draw a circle mat
			uint32_t extent_y = 60;
			uint32_t extent_x = 7;
			cv::Mat circle_mat(extent_y, extent_x, CV_8UC1, cv::Scalar(255));
			uint32_t x_offset = 0;
			uint32_t y_offset = 0;
			uint32_t radius = 3;
			cv::circle(circle_mat, cv::Point(x_offset+radius, y_offset +  radius), radius, cv::Scalar(0),2);
			cv::circle(circle_mat, cv::Point(x_offset+radius, y_offset + radius + (extent_y - 2 * radius) / 3), radius, cv::Scalar(0),2);
			cv::circle(circle_mat, cv::Point(x_offset+radius, y_offset + radius + 2 * (extent_y - 2 * radius) / 3) , radius, cv::Scalar(0),2);
			cv::circle(circle_mat, cv::Point(x_offset+radius, y_offset + (extent_y-radius)), radius, cv::Scalar(0),2);
			//scan a window for the circle
			uint32_t min_deviations = 0xFFFFFFFF;
			int32_t best_x = -999;
			int32_t best_y = -999;
			for (int32_t x_scan = -10; x_scan < 10; ++x_scan)
			{
				for (int32_t y_scan = -10; y_scan < 10; ++y_scan)
				{
					cv::Rect window(ulx + x_scan, uly + y_scan, extent_x, extent_y);
					cv::Mat circles_guess = test_images[image_index](window);
					cv::Mat diff_image = circles_guess - circle_mat;
					cv::Scalar num_deviations = cv::sum(diff_image);
					if (num_deviations[0] < min_deviations)
					{
						min_deviations = num_deviations[0];
						best_x = x_scan;
						best_y = y_scan;
					}
				}
			}
			LOGDEBUG("Best alignment X: %d, best Y: %d", best_x, best_y);
			//mask off the located circles
			//get the region with the circles
			cv::Rect located_answers_rect(ulx + best_x, uly + best_y, extent_x, extent_y);
			cv::Mat located_answers = test_images[image_index](located_answers_rect);
			//redraw the circles as filled in white
			circle_mat = cv::Scalar(0);
			cv::circle(circle_mat, cv::Point(x_offset + radius, y_offset + radius), radius, cv::Scalar(255), -1);
			cv::circle(circle_mat, cv::Point(x_offset + radius, y_offset + radius + (extent_y - 2 * radius) / 3), radius, cv::Scalar(255), -1);
			cv::circle(circle_mat, cv::Point(x_offset + radius, y_offset + radius + 2 * (extent_y - 2 * radius) / 3), radius, cv::Scalar(255), -1);
			cv::circle(circle_mat, cv::Point(x_offset + radius, y_offset + (extent_y - radius)), radius, cv::Scalar(255), -1);
			//and these with the located_answers mat
			//cv::bitwise_and(circle_mat, located_answers, located_answers);
			//check which answer is most black (sums to least
			uint32_t best_answer = 0xFFFFFFFF;
			uint32_t best_answer_at = 0xFFFFFFFF;
			for (uint32_t answer_index = 0; answer_index < 4; ++answer_index)
			{
				cv::Rect single_answer_rect(0, extent_y * answer_index / 4.0, extent_x, extent_y / 4);
				cv::Mat single_answer = located_answers(single_answer_rect);
				cv::Scalar the_sum = cv::sum(single_answer);
				if (best_answer > the_sum[0])
				{
					best_answer = the_sum[0];
					best_answer_at = answer_index;
				}
			}
			//box the selected answer
			LOGDEBUG("Selected answer %d", best_answer_at);
			//cv::circle(located_answers, cv::Point(radius, 2*radius + extent_y / 4.0 * best_answer_at), radius, cv::Scalar(0),-1);
			cv::rectangle(test_images[image_index], cv::Rect(ulx + best_x - extent_x / 2.0, uly + best_y + ((extent_y+5) * best_answer_at / 4.0), extent_x * 2, extent_x * 2),cv::Scalar(128));

			//cv::circle(test_images[image_index], cv::Point(ulx + best_x + radius, uly + best_y + radius), radius, cv::Scalar(0), 2);
			//cv::circle(test_images[image_index], cv::Point(ulx + best_x + radius, uly + best_y + radius + (extent_y - 2 * radius) / 3), radius, cv::Scalar(0), 2);
			//cv::circle(test_images[image_index], cv::Point(ulx + best_x + radius, uly + best_y + radius + 2 * (extent_y - 2 * radius) / 3), radius, cv::Scalar(0), 2);
			//cv::circle(test_images[image_index], cv::Point(ulx + best_x + radius, uly + best_y + (extent_y - radius)), radius, cv::Scalar(0), 2);
			////draw rectangles
			//for (uint32_t sub_index = 0; sub_index < 4; ++sub_index)
			//{

			//	cv::Rect box(ulx, uly + height * sub_index / 4.0, width, height/4.0);
			//	cv::rectangle(test_images[image_index], box, cv::Scalar(0));
			//}
			++region_index;
		}

		page_split_index++;
		cv::imshow("test", test_images[image_index]);
		cv::waitKey(0);
	}

	return 0;
}