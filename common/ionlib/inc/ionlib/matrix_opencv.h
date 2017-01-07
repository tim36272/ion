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
#ifdef OPENCV_CORE_HPP

#ifndef ION_MATRIX_OPENCV_H_
#define ION_MATRIX_OPENCV_H_

#include <opencv2\highgui.hpp>
#include "ionlib\matrix.h"
namespace ion
{
	template <class T>
	void ion::Matrix<T>::imshow()
	{
		cv::Mat image(rows_, cols_, CV_8UC3, data_);
		cv::imshow("matrix", image);
		cv::waitKey(1);
	}
};
#endif //ION_MATRIX_OPENCV_H_
#endif //OPENCV_CORE_HPP
