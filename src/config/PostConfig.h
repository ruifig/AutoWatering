/*
This file should only be included from the config/custom/<CONFIG.H> file being used.


Macro prefixes

AW_xxx - These are macros specific to the AutoWatering project
CZ_xxx - Macros to tweak the czmuc library
TFT_xxx - Macros to control the TFT_eSpi library
*/
#pragma once

/**
 * Serial output settings
 * 
 * AW_USE_CUSTOM_SERIAL
 * Controls if a custom serial should be used. If set to 1, then the AW_CUSTOM_SERIAL, AW_CUSTOM_SERIAL_RXPIN and AW_CUSTOM_SERIAL_TXPIN macros specify what serial object to use.
 * If set to 0, it will use the default Serial object
 * 
*/

// set a default value
#ifndef AW_USE_CUSTOM_SERIAL
	#define AW_USE_CUSTOM_SERIAL 1
#endif

#if AW_USE_CUSTOM_SERIAL
	#ifndef AW_CUSTOM_SERIAL
		#define AW_CUSTOM_SERIAL Serial1
	#endif

	#ifndef AW_CUSTOM_SERIAL_RXPIN 
		#define AW_CUSTOM_SERIAL_RXPIN 17
	#endif

	#ifndef AW_CUSTOM_SERIAL_TXPIN
		#define AW_CUSTOM_SERIAL_TXPIN 16
	#endif
#else
	#define AW_CUSTOM_SERIAL Serial
#endif