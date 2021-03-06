#include "StringUtils.h"
#include <string.h>
#include <stdio.h>

namespace cz
{
	
char* getTemporaryString()
{
	// Use several static strings, and keep picking the next one, so that callers can hold the string for a while without risk of it
	// being changed by another call.
	static char bufs[CZ_TEMPORARY_STRING_MAX_NESTING][CZ_TEMPORARY_STRING_MAX_SIZE];
	static int nBufIndex=0;

	char* buf = bufs[nBufIndex];
	nBufIndex++;
	if (nBufIndex==CZ_TEMPORARY_STRING_MAX_NESTING)
		nBufIndex = 0;

	return buf;
}

const char* formatString(const char* format, ...)
{
	va_list args;
	va_start(args, format);
	const char *str= formatStringVA(format, args);
	va_end(args);
	return str;
}

const char* formatString(const __FlashStringHelper* format, ...)
{
	va_list args;
	va_start(args, format);
	const char *str= formatStringVA(format, args);
	va_end(args);
	return str;
}


char* formatStringVA(const char* format, va_list argptr)
{
	char* buf = getTemporaryString();
	if (vsnprintf(buf, CZ_TEMPORARY_STRING_MAX_SIZE, format, argptr) == CZ_TEMPORARY_STRING_MAX_SIZE)
		buf[CZ_TEMPORARY_STRING_MAX_SIZE-1] = 0;
	return buf;
}

char* formatStringVA(const __FlashStringHelper* format, va_list argptr)
{
	char* buf = getTemporaryString();
#ifdef __AVR__
	if (vsnprintf_P(buf, CZ_TEMPORARY_STRING_MAX_SIZE, (const char*)format, argptr) == CZ_TEMPORARY_STRING_MAX_SIZE) // progmem for AVR
#else
	if (vsnprintf(buf, CZ_TEMPORARY_STRING_MAX_SIZE, format, argptr) == CZ_TEMPORARY_STRING_MAX_SIZE) // for the rest of the world
#endif
	{
		buf[CZ_TEMPORARY_STRING_MAX_SIZE-1] = 0;
	}
	return buf;
}

void strCatPrintf(char* dest, const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	strcat(dest, formatStringVA(fmt, args));
	va_end(args);
}

void strCatPrintf(char* dest, const __FlashStringHelper* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	strcat(dest, formatStringVA(fmt, args));
	va_end(args);
}

char* duplicateChar(char* dest, int n, char ch)
{
	memset(dest, ch, n);
	dest[n] = 0;
	return dest;
}
	
} // namespace cz
