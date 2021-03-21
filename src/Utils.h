#pragma once

#define LOG_ENABLED 1

void logPrintf(const char* fmt, ...);

#if LOG_ENABLED
	#define CZ_LOG(fmt, ...) logPrintf(fmt, ##__VA_ARGS__);
	#define CZ_LOG_LN(fmt, ...) \
		{ \
			logPrintf(fmt, ##__VA_ARGS__); \
			Serial.println(); \
		}
#else
	#define CZ_LOG(fmt, ...)
#endif

void strCatPrintf(char* dest, const char* fmt, ...);


