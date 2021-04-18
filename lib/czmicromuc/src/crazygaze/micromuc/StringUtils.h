#pragma once

#include "crazygaze/micromuc/czmicromuc.h"
#include <stdarg.h>
#include <Arduino.h>

namespace cz
{

#define CZ_TEMPORARY_STRING_MAX_SIZE 128
#define CZ_TEMPORARY_STRING_MAX_NESTING 1

const char* formatString(const char* format, ...);
char* formatStringVA(const char* format, va_list argptr);
const char* formatString(const __FlashStringHelper* format, ...);
char* formatStringVA(const __FlashStringHelper* format, va_list argptr);
void strCatPrintf(char* dest, const char* fmt, ...);
void strCatPrintf(char* dest, const __FlashStringHelper* fmt, ...);
	
	
} // namespace cz