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
#ifndef ION_GFX_H_
#define ION_GFX_H_
namespace ion
{
	class Gfx
	{
	public:
		void Create(const char* title, int width, int height, int x = 0, int y = 0);
		struct WindowHandle_t;
		struct WindowMsg_t;
		struct WindowParam_t;
		struct WindowSecondParam_t;
		struct WindowCallbackResult_t;
		WindowCallbackResult_t* HandleMessage(WindowHandle_t* handle, WindowMsg_t* msg, WindowParam_t* param1, WindowSecondParam_t* param2);
		WindowHandle_t* handle_;
		struct WindowDeviceContext_t;
		WindowDeviceContext_t* device_contex_;
		struct WindowOpenglContext_t;
		WindowOpenglContext_t* opengl_context_;
		struct WindowInstance_t;
		WindowInstance_t* instance_;
	private:
		void CreateOpenGLContext();
	};
};
#endif //ION_GFX_H_