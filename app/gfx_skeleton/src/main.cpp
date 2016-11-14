#include "ionlib\log.h"
#include "ionlib\net.h"
#include "iongfxlib\gfx.h"
#include "iongfxlib\gfx_private.h"
#include "iongfxlib\gfx_opengl.h"
#include <gl/GL.h>
#include <gl/GLU.h>
int main(int argc, char* argv[])
{
	ion::InitSockets();
	ion::LogInit("app");

	ion::Gfx gfx;
	gfx.Create("test", 640, 480);
	ShowWindow(gfx.handle_->hwnd, SW_SHOW);
	ion::InitGraphics();


	MSG msg;

	while (GetMessage(&msg, gfx.handle_->hwnd, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	wglMakeCurrent(NULL, NULL);
	ReleaseDC(gfx.handle_->hwnd, gfx.device_contex_->hdc);
	wglDeleteContext(gfx.opengl_context_->hrc);
	DestroyWindow(gfx.handle_->hwnd);

	return 0;
}