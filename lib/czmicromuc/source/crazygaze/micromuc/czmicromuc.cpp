#include "czmicromuc.h"
#include "Logging.h"
#include "StringUtils.h"
#include <stdarg.h>


namespace cz
{

namespace
{
	#define TEMP_STRING_SIZE 180
	char tempString[TEMP_STRING_SIZE];
}
	
void _doAssert(const char* file, int line, const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	CZ_LOG(logDefault, Error, F("ASSERT: %s:%d"), file, line);
	LogOutput::logToAllSimple(LogVerbosity::Fatal, formatString(F("ASSERT: %d:%d: "), file, line));
	LogOutput::logToAllSimple(LogVerbosity::Fatal, formatStringVA(fmt, args));
	LogOutput::logToAllSimple(LogVerbosity::Fatal, F("\r\n"));
	va_end(args);

	_BREAK();
}

void _doAssert(const char* file, int line, const __FlashStringHelper* fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	CZ_LOG(logDefault, Error, F("ASSERT: %s:%d"), file, line);
	LogOutput::logToAllSimple(LogVerbosity::Fatal, formatString(F("ASSERT: %d:%d: "), file, line));
	LogOutput::logToAllSimple(LogVerbosity::Fatal, formatStringVA(fmt, args));
	LogOutput::logToAllSimple(LogVerbosity::Fatal, F("\r\n"));
	va_end(args);

	_BREAK();
}
	
} // namespace cz


