#pragma once

#include "crazygaze/micromuc/czmicromuc.h"
#include <stdarg.h>
#include <Arduino.h>

#ifndef __AVR__
	#include "avr/dtostrf.h"
#endif

namespace cz
{

#define CZ_TEMPORARY_STRING_MAX_SIZE 128
#define CZ_TEMPORARY_STRING_MAX_NESTING 1

char* getTemporaryString();
const char* formatString(const char* format, ...);
char* formatStringVA(const char* format, va_list argptr);
const char* formatString(const __FlashStringHelper* format, ...);
char* formatStringVA(const __FlashStringHelper* format, va_list argptr);
void strCatPrintf(char* dest, const char* fmt, ...);
void strCatPrintf(char* dest, const __FlashStringHelper* fmt, ...);

/**
 * Creates a null terminated string with n characters (ch)
 * The supplied buffer must be big enough to include n character plus the null terminator
 */
char* duplicateChar(char* dest, int n, char ch);

namespace detail
{

	void skipTo(const char*& src, int c);
	void skipToAfter(const char*& src, int c);
	int advance(const char*& src);
	bool parseParam(const char* src, bool& dst);
	bool parseParam(const char* src, int& dst);
	bool parseParam(const char* src, int8_t& dst);
	bool parseParam(const char* src, uint8_t& dst);
	bool parseParam(const char* src, float& dst);
	bool parseParam(const char* src, char* dst);

	template<typename TFirst>
	bool parse(const char*& src, TFirst& first)
	{
		if (detail::parseParam(src, first))
		{
			detail::advance(src);
			return true;
		}
		else
		{
			return false;
		}
	}

	template<typename TFirst, typename... Args>
	bool parse(const char*& src, TFirst& first, Args&... args)
	{
		if (detail::parseParam(src, first))
		{
			detail::advance(src);
			return parse(src, args...);
		}
		else
		{
			return false;
		}
	}
};

template<typename TFirst, typename... Args>
bool parse(const char*& src, TFirst& first, Args&... args)
{
	return detail::parse(src, first, args...);
}

template<int BufSize=20, int MinWidth=0, int Precision=3> 
struct FloatToString
{
	inline explicit FloatToString(float val)
	{
		dtostrf(val, MinWidth, Precision, str);
	}

	inline const char* operator*() const
	{
		return str;
	}

	char str[BufSize];
};

struct IntToString
{
	inline explicit IntToString(int32_t value)
	{
		itoa(value, str, 10);
	}

	inline const char* operator*() const
	{
		return str;
	}

	// 12 characters is enough to represent a 32bits number [-2147483648 to 2147483647]
	char str[12];
};




	
} // namespace cz