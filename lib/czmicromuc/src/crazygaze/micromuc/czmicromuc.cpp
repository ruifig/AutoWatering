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

const char* getFilename(const char* file)
{
	const char* a = strrchr(file, '\\');
	const char* b = strrchr(file, '/');
	const char* c = std::max(a, b);
	return c ? c+1 : file;
}

	
void _doAssert(const char* file, int line, const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	LogOutput::logToAllSimple(formatString(F("ASSERT: %s:%d: "), file, line));
	LogOutput::logToAllSimple(formatStringVA(fmt, args));
	LogOutput::logToAllSimple(F("\r\n"));
	va_end(args);

	delay(1000);
	_BREAK();
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

	_BREAK();
}
	
} // namespace cz


