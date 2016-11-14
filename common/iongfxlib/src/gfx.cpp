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
#include "iongfxlib/gfx.h"
#include <windows.h>			/* must include this before GL/gl.h */
#include <GL/gl.h>			/* OpenGL header file */
#include <GL/glu.h>			/* OpenGL utilities header file */
#include <stdio.h>
#include "ionlib/log.h"
#include "iongfxlib/gfx_private.h"
namespace ion
{
	LRESULT CALLBACK GenericWndProc(HWND hWnd, UINT nMessage, WPARAM wParam, LPARAM lParam)
	{
		ion::Gfx *ptrWindow = NULL;

		// Get pointer to window
		if (nMessage == WM_NCCREATE)
		{
			SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)((LPCREATESTRUCT(lParam))->lpCreateParams));
		}

		// Get pointer to window
		ptrWindow = (ion::Gfx*)GetWindowLongPtr(hWnd, GWLP_USERDATA);

		// Foward message to appropriate window procedure
		if (ptrWindow)
		{
			return (LRESULT)ptrWindow->HandleMessage((Gfx::WindowHandle_t*)&hWnd, (Gfx::WindowMsg_t*)&nMessage, (Gfx::WindowParam_t*)&wParam, (Gfx::WindowSecondParam_t*)&lParam);
		}

		return DefWindowProc(hWnd, nMessage, wParam, lParam);
	}
	void ion::Gfx::Create(const char* title, int width, int height, int x, int y)
	{
		int         pf;
		HDC         hDC;
		WNDCLASS    wc;
		PIXELFORMATDESCRIPTOR pfd;
		static HINSTANCE hInstance = 0;

		handle_ = new(WindowHandle_t);
		/* only register the window class once - use hInstance as a flag. */
		if (!hInstance)
		{
			hInstance = GetModuleHandle(NULL);
			wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC | CS_DBLCLKS;
			wc.lpfnWndProc = (WNDPROC)GenericWndProc;
			wc.cbClsExtra = 0;
			wc.cbWndExtra = 0;
			wc.hInstance = hInstance;
			wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);
			wc.hCursor = NULL; //LoadCursor(NULL, IDC_ARROW);
			wc.hbrBackground = NULL;
			wc.lpszMenuName = NULL;
			wc.lpszClassName = "Window";

			if (!RegisterClass(&wc))
			{
				MessageBox(NULL, "RegisterClass() failed:  "
						   "Cannot register window class.", "Error", MB_OK);
				LOGFATAL("See msgbox");
			}
		}

		handle_->hwnd = CreateWindow("Window", title, WS_OVERLAPPEDWINDOW |
									 WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
									 x, y, width, height, NULL, NULL, hInstance, (void*)this);

		if (handle_->hwnd == NULL)
		{
			MessageBox(NULL, "CreateWindow() failed:  Cannot create a window.",
					   "Error", MB_OK);
			LOGFATAL("See msgbox");
		}

		hDC = GetDC(handle_->hwnd);

		/* there is no guarantee that the contents of the stack that become
		the pfd are zeroed, therefore _make sure_ to clear these bits. */
		memset(&pfd, 0, sizeof(pfd));
		pfd.nSize = sizeof(pfd);
		pfd.nVersion = 1;
		pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER | PFD_TYPE_RGBA;
		pfd.iPixelType = PFD_TYPE_RGBA;
		pfd.cColorBits = 32;
		pfd.cDepthBits = 32;
		pfd.cStencilBits = 1;
		pfd.iLayerType = PFD_MAIN_PLANE;

		pf = ChoosePixelFormat(hDC, &pfd);
		if (pf == 0)
		{
			MessageBox(NULL, "ChoosePixelFormat() failed:  "
					   "Cannot find a suitable pixel format.", "Error", MB_OK);
			LOGFATAL("See msgbox");
		}

		if (SetPixelFormat(hDC, pf, &pfd) == FALSE)
		{
			MessageBox(NULL, "SetPixelFormat() failed:  "
					   "Cannot set format specified.", "Error", MB_OK);
			LOGFATAL("See msgbox");
		}

		//DescribePixelFormat(hDC, pf, sizeof(PIXELFORMATDESCRIPTOR), &pfd);

		//ReleaseDC(hDC, hWnd);
		CreateOpenGLContext();
	}
	bool SetupPixelFormat(HDC& hdc)
	{
		static PIXELFORMATDESCRIPTOR pfd =				// pfd tells Windows how we want "things" to be
		{
			sizeof(PIXELFORMATDESCRIPTOR),				// Size of this pixel format descriptor
			1,											// Version number
			PFD_DRAW_TO_WINDOW |						// Format must support window
			PFD_SUPPORT_OPENGL |						// Format must support OpenGL
			PFD_DOUBLEBUFFER,							// Must support double buffering
			PFD_TYPE_RGBA,								// Request an RGBA format
			32,											// Select our color depth
			0, 0, 0, 0, 0, 0,							// Color bits ignored
			0,											// No alpha buffer
			0,											// Shift bit ignored
			0,											// No accumulation buffer
			0, 0, 0, 0,									// Accumulation bits ignored
			32,											// 32 Bit z-buffer (depth buffer)  
			1,											// Stencil buffer
			0,											// No auxiliary buffer
			PFD_MAIN_PLANE,								// Main drawing layer
			0,											// Reserved
			0, 0, 0										// Layer masks ignored
		};

		int PixelFormat;
		if (!(PixelFormat = ChoosePixelFormat(hdc, &pfd)))	// Did windows find a matching pixel format?
		{
			LOGFATAL("Could not find a suitable pixel format");
			return false;
		}

		if (!SetPixelFormat(hdc, PixelFormat, &pfd))	// Are we able to set the pixel format?
		{
			LOGFATAL("Could not set pixel format");
			return false;
		}

		return true;
	}
	void Gfx::CreateOpenGLContext()
	{
		//make sure the handle points to a valid control    
		if (handle_->hwnd == NULL)
		{
			MessageBox(NULL, "ERROR: Could not find control handle.", "Error", MB_OK | MB_ICONERROR);
			LOGFATAL("See msgbox");
		}

		//atempt to create a device context    
		device_contex_ = new(Gfx::WindowDeviceContext_t);
		device_contex_->hdc = GetDC(handle_->hwnd);
		if (device_contex_->hdc == NULL)
		{
			MessageBox(NULL, "ERROR: Could not create device context.", "Error", MB_OK | MB_ICONERROR);
			LOGFATAL("See msgbox");
		}

		//attempt to get a pixel format    
		if (!SetupPixelFormat(device_contex_->hdc))
		{
			MessageBox(NULL, "ERROR: Could not set pixel format.", "Error", MB_OK | MB_ICONERROR);
			LOGFATAL("See msgbox");
		}

		//attempt to initialize the rendering context

		opengl_context_->hrc = wglCreateContext(device_contex_->hdc);
		if (opengl_context_->hrc == NULL)
		{
			MessageBox(NULL, "ERROR: Could not create rendering context.", "Error", MB_OK | MB_ICONERROR);
			LOGFATAL("See msgbox");
		}

		//make this the current context
		if (!wglMakeCurrent(device_contex_->hdc, opengl_context_->hrc))
		{
			MessageBox(NULL, "ERROR: Could not activate rendering context.", "Error", MB_OK | MB_ICONERROR);
			LOGFATAL("See msgbox");
		}

	}
	void
		display()
	{
		/* rotate a triangle around */
		glClear(GL_COLOR_BUFFER_BIT);
		glBegin(GL_TRIANGLES);
		glColor3f(1.0f, 0.0f, 0.0f);
		glVertex2i(0, 1);
		glColor3f(0.0f, 1.0f, 0.0f);
		glVertex2i(-1, -1);
		glColor3f(0.0f, 0.0f, 1.0f);
		glVertex2i(1, -1);
		glEnd();
		glFlush();
	}
	Gfx::WindowCallbackResult_t* Gfx::HandleMessage(WindowHandle_t * handle, WindowMsg_t * msg, WindowParam_t * param1, WindowSecondParam_t * param2)
	{
		static PAINTSTRUCT ps;

		switch (msg->msg)
		{
			case WM_PAINT:
				display();
				BeginPaint(handle->hwnd, &ps);
				EndPaint(handle->hwnd, &ps);
				return 0;

			case WM_SIZE:
				glViewport(0, 0, LOWORD(param2->lParam), HIWORD(param1->wParam));
				PostMessage(handle->hwnd, WM_PAINT, 0, 0);
				return 0;

			case WM_CHAR:
				switch (param1->wParam)
				{
					case 27:			/* ESC key */
						PostQuitMessage(0);
						break;
				}
				return 0;

			case WM_CLOSE:
				PostQuitMessage(0);
				return 0;
		}

		return (Gfx::WindowCallbackResult_t*)DefWindowProc(handle->hwnd, msg->msg, param1->wParam, param2->lParam);
	}
};