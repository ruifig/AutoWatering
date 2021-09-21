#pragma once

#include "crazygaze/micromuc/czmicromuc.h"
#include <stdarg.h>
#include <Arduino.h>

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


struct detail
{

	static void skipTo(const char*& src, int c)
	{
		while (*src && *src != c)
		{
			++src;
		}
	}

	static void skipToAfter(const char*& src, int c)
	{
		skipTo(src, c);
		if (*src && *src == c)
		{
			++src;
		}
	}


	static int advance(const char*& src)
	{
		const char* start = src;

		if (*src == '"')
		{
			++src;
			skipToAfter(src, '"');
		}
		else
		{
			skipTo(src, ' ');
		}

		int size = src - start;

		while (*src && *src == ' ')
		{
			++src;
		}

		return size;
	}

	static bool parseParam(const char* src, int& dst)
	{
		int c = *src;
		if (c == '+' || c == '-' || (c >= '0' && c <= '9'))
		{
			dst = atoi(src);
			return true;
		}
		else
		{
			return false;
		}
	}

	static bool parseParam(const char* src, float& dst)
	{
		int c = *src;
		if (c=='.' || c == '+' || c == '-' || (c >= '0' && c <= '9'))
		{
			dst = static_cast<float>(atof(src));
			return true;
		}
		else
		{
			return false;
		}
	}

	static bool parseParam(const char* src, char* dst)
	{
		const char* ptr = src;
		int size = advance(ptr);
		if (*src == '"')
		{
			ptr = src+1;
			size -= 2;
		}
		else
		{
			ptr = src;
		}

		memcpy(dst, ptr, size);
		dst[size] = 0;
		return true;
	}

	template<typename TFirst, typename... Args>
	static bool parse(const char*& src, TFirst& first, Args&... args)
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

	template<typename TFirst>
	static bool parse(const char*& src, TFirst& first)
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


	
} // namespace cz