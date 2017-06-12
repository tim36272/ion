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
#ifndef FFWRAPPER_WRITE_H_
#define FFWRAPPER_WRITE_H_
#include "ffwrapper\common.h"
#include <string>
namespace ion
{
	struct FFWriteImpl;
	class FFWriter
	{
	public:
		FFWriter(std::string uri, uint32_t rows, uint32_t cols, const char* codec_name = nullptr);
		~FFWriter();
		void WriteFrame(const ion::Image& img);
		void Close();
	private:
		FFWriteImpl* impl;
		bool video_open_;
	};
};
#endif //FFWRAPPER_WRITE_H_