#pragma once

#include <Arduino.h>

#if defined(DEBUG) || defined(_DEBUG)
	#define CZ_DEBUG 1
#else
	
	#if !defined(NDEBUG)
		#error No _DEBUG/DEBUG or NDEBUG defined
	#endif

	#define CZ_DEBUG 0
#endif

#if !defined(CZ_LOG_ENABLED)
	#define CZ_LOG_ENABLED 1
#endif

#if !defined(CZ_SERIAL_LOG_ENABLED)
	#if CZ_DEBUG
		#define CZ_SERIAL_LOG_ENABLED 1
	#else
		#define CZ_SERIAL_LOG_ENABLED 0
	#endif
#endif

// Removes path from __FILE__
// Copied from https://stackoverflow.com/questions/8487986/file-macro-shows-full-path
#define __FILENAME__ (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)

#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__) || defined(__AVR_ATmega1284__) || defined(__AVR_ATmega1284P__) || defined(__AVR_ATmega644__) || defined(__AVR_ATmega644A__) || defined(__AVR_ATmega644P__) || defined(__AVR_ATmega644PA__)
	#define _BREAK() __asm__ __volatile__("break")
#endif

//
// assert macros
//
#if CZ_DEBUG
	#define CZ_ASSERT(expression) if (!(expression)) { ::cz::_doAssert(__FILENAME__, __LINE__, #expression); }
	#define CZ_UNEXPECTED() ::cz::_doAssert(__FILENAME__, __LINE__, "Unexpected code path")
#else
	#define CZ_ASSERT(expression) ((void)0)
	#define CZ_UNEXPECTED() ((void)0)
#endif

namespace cz
{
	void _doAssert(const char* file, int line, const char* fmt, ...) __attribute__ ((format (printf, 3,4)));
	void _doAssert(const char* file, int line, const __FlashStringHelper* fmt, ...);
}

