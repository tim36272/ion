#ifndef ION_APP_UTIL_H_
#define ION_APP_UTIL_H_
#include <stdint.h>
namespace ion
{
	bool AppInit(const char* log_file_name);
	void AppClose();
	void AppFail(int32_t result);
} //namespace ion
#endif //IONAPPUTIL_H_