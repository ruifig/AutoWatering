#include "czmicromuc.h"
#include "Logging.h"
#include "StringUtils.h"
#include "algorithm"
#include <stdarg.h>


namespace cz
{

namespace
{
	#define TEMP_STRING_SIZE 180
	char tempString[TEMP_STRING_SIZE];
}

#if defined(ARDUINO)
const __FlashStringHelper* getFilename(const __FlashStringHelper* file_)
{
	const char* file= reinterpret_cast<const char*>(file_);
	const char* a = strrchr_P(file, '\\');
	const char* b = strrchr_P(file, '/');
	const char* c = a > b ? a : b;
	return reinterpret_cast<const __FlashStringHelper*>(c ? c+1 : file);
}
#else
const char* getFilename(const char* file)
{
	const char* a = strrchr(file, '\\');
	const char* b = strrchr(file, '/');
	const char* c = a > b ? a : b;
	return c ? c+1 : file;
}
#endif


#if if
void _doAssert(const char* file, int line, const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	LogOutput::logToAllSimple(formatString(F("ASSERT: %s:%d: "), file, line));
	LogOutput::logToAllSimple(formatStringVA(fmt, args));
	LogOutput::logToAllSimple(F("\r\n"));
	va_end(args);

	LogOutput::flush();
	_BREAK();
	while(true) {}
}

void _doAssert(const char* file, int line, const __FlashStringHelper* fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	CZ_LOG(logDefault, Error, F("ASSERT: %s:%d"), file, line);
	LogOutput::logToAllSimple(formatString(F("ASSERT: %d:%d: "), file, line));
	LogOutput::logToAllSimple(formatStringVA(fmt, args));
	LogOutput::logToAllSimple(F("\r\n"));
	va_end(args);

	LogOutput::flush();
	_BREAK();
	while(true) {}
}

#endif

void _doAssert(const __FlashStringHelper* file, int line, const __FlashStringHelper* fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	LogOutput::logToAllSimple(F("ASSERT: "));
	LogOutput::logToAllSimple(file);
	LogOutput::logToAllSimple(formatString(F(":%d:: "), line));
	LogOutput::logToAllSimple(formatStringVA(fmt, args));
	LogOutput::logToAllSimple(F("\r\n"));
	va_end(args);

	LogOutput::flush();
	_BREAK();
	while(true) {}
}
	
} // namespace cz


