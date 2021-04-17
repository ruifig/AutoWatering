#pragma once

#include "czMicroMuc.h"

namespace cz
{

#define CZ_TEMPORARY_STRING_MAX_SIZE 128
#define CZ_TEMPORARY_STRING_MAX_NESTING 1

const char* formatString(const char* format, ...);
char* formatStringVA(const char* format, va_list argptr);
void strCatPrintf(char* dest, const char* fmt, ...);
	
	
} // namespace cz