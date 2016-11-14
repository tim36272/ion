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
#include <stdint.h>
#include <GL/glew.h>			/* OpenGL utilities header file */
#include "ionlib/log.h"
namespace ion
{
	bool InitGraphics()
	{
		GLenum ret = glewInit();
		if (ret != GLEW_OK)
		{
			LOGFATAL("Unable to initialize GLEW: %s", glewGetErrorString(ret));
		} else
		{
			LOGINFO("Initialized GLEW: %s", glewGetString(GLEW_VERSION));

			const GLubyte* ext = glGetString(GL_VERSION);
			LOGINFO("OpenGL Version: %s", ext);
		}

		return true;
	}
	void CheckGLError(const char* file, uint32_t line)
	{
		GLenum errNum = glGetError();
		while (errNum != GL_NO_ERROR)
		{
			LOGFATAL("%s (%i) OpenGL Error: %s", file, line, gluErrorString(errNum));
			errNum = glGetError();
		}
	}
};