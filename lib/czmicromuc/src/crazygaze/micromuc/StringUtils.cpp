#include "StringUtils.h"
#include <string.h>
#include <stdio.h>

namespace cz
{

namespace detail
{

	void skipTo(const char*& src, int c)
	{
		while (*src && *src != c)
		{
			++src;
		}
	}

	void skipToAfter(const char*& src, int c)
	{
		skipTo(src, c);
		if (*src && *src == c)
		{
			++src;
		}
	}

	int advance(const char*& src)
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

	bool parseParam(const char* src, bool& dst)
	{
		int dummy;
		if (parseParam(src, dummy))
		{
			dst = dummy==0 ? false : true;
			return true;
		}
		else
		{
			return false;
		}
	}

	bool parseParam(const char* src, int& dst)
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

	bool parseParam(const char* src, int8_t& dst)
	{
		int val;
		bool res = parseParam(src, val);
		if (res && val>=-128 && val<=127)
		{
			dst = static_cast<int8_t>(val);
			return true;
		}
		else
		{
			return false;
		}
	}

	bool parseParam(const char* src, uint8_t& dst)
	{
		int val;
		bool res = parseParam(src, val);
		if (res && val>=0 && val<=255)
		{
			dst = static_cast<uint8_t>(val);
			return true;
		}
		else
		{
			return false;
		}
	}

	bool parseParam(const char* src, float& dst)
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

	bool parseParam(const char* src, char* dst)
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
}

	
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
#ifdef __AVR__
	char* buf = getTemporaryString();
	if (vsnprintf_P(buf, CZ_TEMPORARY_STRING_MAX_SIZE, (const char*)format, argptr) == CZ_TEMPORARY_STRING_MAX_SIZE) // progmem for AVR
	{
		buf[CZ_TEMPORARY_STRING_MAX_SIZE-1] = 0;
	}
	return buf;
#else
	return formatStringVA(reinterpret_cast<const char*>(format), argptr);
#endif
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
