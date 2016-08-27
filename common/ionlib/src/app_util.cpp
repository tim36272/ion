#include "ionlib\app_util.h"
#include "ionlib\log.h"
#include "ionlib\net.h"
#include <intrin.h>
#include <stdio.h>
namespace ion
{
	bool AppInit(const char* log_file_name)
	{
		bool result = true;
		if (!ion::LogInit(log_file_name))
		{
			result = false;
		}
		return result;
	}
	void AppClose()
	{
		ion::LogClose();
	}
	void AppFail(int32_t result)
	{
		ion::LogClose();
#ifndef NDEBUG
		__debugbreak();
#endif
		fflush(stdout);
		fflush(stderr);
		exit(result);
	}
} //namespace ion