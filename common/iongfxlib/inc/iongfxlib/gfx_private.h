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
#ifndef ION_GFX_PRIVATE_H_
#define ION_GFX_PRIVATE_H_
#include <windows.h>			/* must include this before GL/gl.h */
#include "iongfxlib\gfx.h"
namespace ion
{
	struct Gfx::WindowHandle_t
	{
		HWND hwnd;
	};
	struct Gfx::WindowMsg_t
	{
		UINT msg;
	};
	struct Gfx::WindowParam_t
	{
		WPARAM wParam;
	};
	struct Gfx::WindowSecondParam_t
	{
		LPARAM lParam;
	};
	struct Gfx::WindowCallbackResult_t
	{
		LRESULT result;
	};
	struct Gfx::WindowDeviceContext_t
	{
		HDC hdc;
	};
	struct Gfx::WindowOpenglContext_t
	{
		HGLRC hrc;
	};
};
#endif //ION_FILE_H_