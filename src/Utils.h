#pragma once

#include <string.h>
#include <Arduino.h>

#ifdef _DEBUG
	#define LOG_ENABLED 1
	#define ASSERT_ENABLED 1
#else
	#define LOG_ENABLED 0
	#define ASSERT_ENABLED 0
#endif

// Removes path from __FILE__
// Copied from https://stackoverflow.com/questions/8487986/file-macro-shows-full-path
#define __FILENAME__ (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)


namespace cz
{
	void logPrintf(const char* fmt, ...);
	void logPrintln();
	void strCatPrintf(char* dest, const char* fmt, ...);
}

#if LOG_ENABLED
	#define CZ_LOG(fmt, ...) cz::logPrintf(fmt, ##__VA_ARGS__);
	#define CZ_LOG_LN(fmt, ...) \
		{ \
			cz::logPrintf(fmt, ##__VA_ARGS__); \
			cz::logPrintln(); \
		}
#else
	#define CZ_LOG(fmt, ...) ((void)0)
	#define CZ_LOG_LN(fmt, ...) ((void)0)
#endif


#if ASSERT_ENABLED
	#define CZ_ASSERT(expression) if (!(expression)) { CZ_LOG("ASSERT: %s:%d", __FILENAME__, __LINE__); delay(1000); abort(); }
#else
	#define CZ_ASSERT(expression) ((void)0)
#endif

#define CZ_UNEXPECTED() CZ_ASSERT("Unexpected" && 0)

