#include "Utils.h"
#include <Arduino.h>

char tempString[256];

void logPrintf(const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	vsnprintf(tempString, 256, fmt, args);
	Serial.print(tempString);
	va_end(args);
}

void strCatPrintf(char* dest, const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	vsnprintf(tempString, 256, fmt, args);
	va_end(args);
	strcat(dest, tempString);
}

