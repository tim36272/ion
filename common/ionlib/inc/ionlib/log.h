#ifndef ION_LOG_H_
#define ION_LOG_H_
#include <stdint.h>
#include <stdlib.h>
#include "app_util.h"

#define IONLOG_MAX_MESSAGE_LENGTH 512

namespace ion
{
	//define macros for debug, info, warn, error, and fatal types
	bool LogInit(const char* log_file_name);
	void LogPrintf(const char* file, uint32_t line, char* format, ...);
	void LogClose();

#define LOGDEBUG(...) ion::LogPrintf(__FILE__, __LINE__, "DEBUG: " ## __VA_ARGS__)
#define LOGINFO(...)  ion::LogPrintf(__FILE__, __LINE__, "INFO: " ## __VA_ARGS__)
#define LOGWARN(...)  ion::LogPrintf(__FILE__, __LINE__, "WARN: " ## __VA_ARGS__)
#define LOGERROR(...) ion::LogPrintf(__FILE__, __LINE__, "ERROR: " ## __VA_ARGS__)
#define LOGFATAL(...) {ion::LogPrintf(__FILE__, __LINE__, "FATAL: " ## __VA_ARGS__); ion::AppFail(-1);}

} //namespace ion
#endif //IONLOG_H_