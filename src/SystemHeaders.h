#pragma once

#pragma GCC system_header

#pragma GCC push_options
#pragma GCC diagnostic push

#pragma GCC diagnostic ignored "-Wcpp"
#pragma GCC diagnostic ignored "-Wextra"

#ifndef __cplusplus
	#ifndef __OPTIMIZE__
		#define __OPTIMIZE__
		#pragma GCC optimize ("Os")
		#include <util/delay.h>
		#undef __OPTIMIZE__
	#else
		#include <Arduino.h>
	#endif
#endif

#pragma GCC diagnostic pop
#pragma GCC pop_options
