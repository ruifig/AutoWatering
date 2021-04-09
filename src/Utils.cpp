#include "Utils.h"
#include <Arduino.h>

/*
 * If using avr-stub, then we can't use Serial.print
 */ 
#ifdef AVR8_BREAKPOINT_MODE
#include "avr8-stub.h"
#endif

namespace cz
{

namespace
{
	char tempString[256];
}

void logPrintf(const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	vsnprintf(tempString, 256, fmt, args);
#ifdef AVR8_BREAKPOINT_MODE
	debug_message(tempString);
#else
	Serial.print(tempString);
#endif
	va_end(args);
}

void logPrintln()
{
#ifdef AVR8_BREAKPOINT_MODE
	debug_message("\n");
#else
	Serial.println();
#endif
}

void strCatPrintf(char* dest, const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	vsnprintf(tempString, 256, fmt, args);
	va_end(args);
	strcat(dest, tempString);
}

} // namespace cz
