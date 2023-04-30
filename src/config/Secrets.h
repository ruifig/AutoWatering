#pragma once

#ifndef AW_CONFIG_DONE
	#error Secrets.h should only be included from PostConfig.h. Refer to the documentation to see how to create configurations.
#endif

#if __has_include("sensitive/MySecrets.h")
	#include "sensitive/MySecrets.h"
#else
	#error "MySecrets.h not found. Read Secrets.h for more information."
#endif

/**
 
 Some things you do NOT want to put on public github projects, such as your Adafruit IO key(s), or your Wifi's ssid and password.
 AutoWatering expects a "MySecrets.h" file to exist that provides those details.

 WARNING : Make sure you do NOT share your own "MySecrets.h" file by mistake.
*/

// If wifi use is enabled, then make sure all required defines exist
#if AW_WIFI_ENABLED
	#ifndef WIFI_SSID
		#error WIFI_SSID not defined.
	#endif

	#ifndef WIFI_PASSWORD
		#error WIFI_PASSWORD not defined.
	#endif

	#ifndef ADAFRUIT_IO_USERNAME
		#error ADAFRUIT_IO_USER not defined.
	#endif

	#ifndef ADAFRUIT_IO_KEY
		#error ADAFRUIT_IO_KEY not defined.
	#endif
#endif
