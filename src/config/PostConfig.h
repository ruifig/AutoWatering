/*
This file should only be included from the config/custom/<CONFIG.H> file being used.

Macro prefixes

AW_xxx - These are macros specific to the AutoWatering project
CZ_xxx - Macros to tweak the czmuc library
TFT_xxx - Macros to control the TFT_eSpi library
*/
#pragma once


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                               LOGGING AND SERIAL PORT OPTIONS
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
Logging can have several outputs, such as
	- SD card: Currently not working. It was working in initial versions when I was using different hardware
	- Serial output: Can be tweaked to specify what uart to use
*/

/*
AW_USE_CUSTOM_SERIAL
Controls if a custom serial should be used. If set to 1, then the AW_CUSTOM_SERIAL, AW_CUSTOM_SERIAL_RXPIN and AW_CUSTOM_SERIAL_TXPIN macros specify what serial object to use.
If set to 0, it will use the default Serial object

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

/*
If set to 1, it enables logging (the CZ_LOG macros do something)
If set to 0, it completely disables logging (the CZ_LOG macros will be compiled out)
*/
#ifndef CZ_LOG_ENABLED
	#if AW_DEBUG
		#define CZ_LOG_ENABLED 1
	#else
		#define CZ_LOG_ENABLED 0
	#endif
#endif

/*
If set to 1, it will enable logging to serial. CZ_LOG_ENABLED must also be set
*/
#ifndef CZ_SERIAL_LOG_ENABLED
	// If not defined, then we set to 1 or 0 depending on CZ_LOG_ENABLED

	#if CZ_LOG_ENABLED
		#define CZ_SERIAL_LOG_ENABLED 1
	#else
		#define CZ_SERIAL_LOG_ENABLED 0
	#endif
#else
	// If specified and 1, then we need to check if CZ_LOG_ENABLED is also set to 1
	#if CZ_SERIAL_LOG_ENABLED && !CZ_LOG_ENABLED
		#error CZ_LOG_ENABLED needs to be 1 to be able to set CZ_SERIAL_LOG_ENABLED to 1
	#endif
#endif

/*
If 1, it will enable logging to SD card.
WARNING: SD card logging is untested. It was working a long time ago when I was using an Arduino Mega 2560, but hasn't been tested for a very long time.
*/
#ifndef AW_SD_CARD_LOGGING
	#define AW_SD_CARD_LOGGING 0
#endif

/*
What pin to as CS/SS for the SD card reader
*/
//#define SD_CARD_SS_PIN 53

/*
If 1, it enabled the CommandConsole component
*/
#ifndef AW_COMMAND_CONSOLE_ENABLED
	#if AW_DEBUG && CZ_SERIAL_LOG_ENABLED
		#define AW_COMMAND_CONSOLE_ENABLED 1
	#else
		#define AW_COMMAND_CONSOLE_ENABLED 0
	#endif
#else
	// If set to 1, make sure the other required macros are set accordingly
	#if AW_COMMAND_CONSOLE_ENABLED && !CZ_SERIAL_LOG_ENABLED
		#error CZ_SERIAL_LOG_ENABLED needs to be set to 1 to be able to set AW_COMMAND_CONSOLE_ENABLED
	#endif
#endif


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                               NETWORK OPTIONS
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
 If 0, no wifi is used
 This will disable any features that require a network connection.
 If set to 1, make sure you provide your network details. See Config/Secrets.h for more information
 */
#define AW_WIFI_ENABLED 1

/*
How long to wait between attempts to reconnect to the mqtt broker
*/
#define AW_MQTT_CONNECTION_RETRY_INTERVAL 5.0f

/**
At the time of writting, I've noticed that sometimes even if Wifi is supposed to be connected, establishing TCP connections fails, and never recovers.
Setting this to 0 means no disconnect/reconnect is done
Setting this to N (where N>0), will perform a wifi disconnect/reconnect after failing to establish the TCP connection N times.

WARNING: If the Watchdog is enabled (see AW_AW_WATCHDOG_ENABLED), then a wifi reconnect attempt will very likely trigger the watchdog.
 */
#define AW_MQTT_WIFI_RECONNECT 5


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                               WATCHDOG COMPONENT OPTIONS
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
If set to 1, it will enable the Watchdog component
See https://arduino-pico.readthedocs.io/en/latest/rp2040.html#hardware-watchdog for limitations
*/

#ifndef AW_WATCHDOG_ENABLE
	#define AW_WATCHDOG_ENABLED 1
#endif

/*
The RP2040 watchdog can't be stopped once started, which can be a problem when making calls to code that takes too long to complete.
For example, connecting to WiFi can take longer than 8300ms, which causes the Watchdog to trigger.
One way to work around this is to fake a watchdog pause by automatically ticking in a separate FreeRTOS task when we need to cal such code.
It works fine, but at the moment of writing there are a few problems using FreeRTOS with Arduino-Pico, such as problems using WiFi.
See: https://github.com/earlephilhower/arduino-pico/pull/1395

As such, it's recommend this stays set to 0 so FreeRTOS is not used.
*/
#ifndef AW_WATCHDOG_PAUSE_SUPPORT
	#define AW_WATCHDOG_PAUSE_SUPPORT 0
#endif


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                               BATTERYLIFE COMPONENT OPTIONS
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
If set to 1, it will enable the BatteryLife component, which uses a voltage divider to calculate the battery voltage
*/
#ifndef AW_BATTERYLIFE_ENABLED
	#define AW_BATTERYLIFE_ENABLED 1
#endif

#if AW_BATTERYLIFE_ENABLED

	// What analog pin to use for battery voltage readings
	#ifndef AW_BATTERYLIFE_PIN
		#define AW_BATTERYLIFE_PIN cz::MCUPin(26)
	#endif

	// R1 value in ohm
	#ifndef AW_BATTERYLIFE_R1_VALUE
		#define AW_BATTERYLIFE_R1_VALUE 267200
	#endif

	// R2 value in ohm
	#ifndef AW_BATTERYLIFE_R2_VALUE
		#define AW_BATTERYLIFE_R2_VALUE 459600
	#endif

	// What voltage (float number) below which we should consider the battery has 0% charge
	#ifndef AW_BATTERYLIFE_VBAT_0
		#define AW_BATTERYLIFE_VBAT_0 2.5f
	#endif

	// What voltage (float number) above which we should consider the battery has 100% charge
	#ifndef AW_BATTERYLIFE_VBAT_100
		#define AW_BATTERYLIFE_VBAT_100 4.2f
	#endif

	// A scaling adjustment to add to the voltage readings.
	// Because I'm such a noob and still learning, I couldn't figure how to get the voltage divider to give the correct values.
	// It is consistently lower than expected. Maybe because of what load is on the circuit? 
	// I'll come back to this eventually, but for my use case, applying this scaling seems to work ok.
	#ifndef AW_BATTERYLIFE_VBAT_ADJUSTMENT
		#define AW_BATTERYLIFE_VBAT_ADJUSTMENT 1.07f
	#endif

#endif

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                               LED STATUS COMPONENT OPTIONS
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
If set to 1, it enabled the status led.
In short, it uses the builtin led to signal things so we know the board it's alive and not frozen
*/
#ifndef AW_LEDSTATUS_ENABLED
	#define AW_LEDSTATUS_ENABLED 1
#endif


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                               TEMPERATURE&HUMIDITY COMPONENT OPTIONS
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
If set to 1, it will enable the TemperatureAndHumiditySensor component
At the moment, only the HTU21D sensor is supported
*/
#ifndef AW_THSENSOR_SENSOR_ENABLED
	#define AW_THSENSOR_SENSOR_ENABLED 1
#endif

// Default sampling interval, in seconds
#if AW_FASTER_ITERATION
	#define AW_THSENSOR_DEFAULT_SAMPLINGINTERVAL 60.0f
#else
	#define AW_THSENSOR_DEFAULT_SAMPLINGINTERVAL 120.0f
#endif

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                               SOIL MOISTURE SENSOR COMPONENT OPTIONS
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
When we want to take a moisture reading, we enable power to the the sensor and need to wait a bit before doing the
actual reading.
This specifies how many seconds to wait before doing the reading
*/
#ifndef AW_MOISTURESENSOR_POWERUP_WAIT
	#define AW_MOISTURESENSOR_POWERUP_WAIT 0.20f
#endif

/*
Default sensor sampling interval in seconds. Needs to be integer number.
Note that internally the sampling interval is tracked in seconds, but the UI shows it in minutes. This is because
allowing it in seconds is easier for development since we can have the sensor reacting fast, BUT showing the setting's UI in seconds
would be cumbersome since the user will probably want big intervals between samplings (intervals in minutes probably)
*/
#ifndef AW_MOISTURESENSOR_DEFAULT_SAMPLINGINTERVAL
	#if AW_FASTER_ITERATION
		#define AW_MOISTURESENSOR_DEFAULT_SAMPLINGINTERVAL 1
	#else
		#define AW_MOISTURESENSOR_DEFAULT_SAMPLINGINTERVAL 60
	#endif
#endif

/*
How many sensors can be active at one given time
Depending on the board design, sensor readings might be sharing a single arduino pin through a mux, in which case this needs to be set to 1.
Also, if the sensors are using pins to supply power, having this set to 1 can lower peak power.
*/
#ifndef AW_MAX_SIMULTANEOUS_MOISTURESENSORS
	#define AW_MAX_SIMULTANEOUS_MOISTURESENSORS 1
#endif

/*
Maximum sampling interval allowed in seconds (integer number)
This also limits the the maximum value the UI will allow and show.
Please note that the UI shows sampling intervals in minutes, but the internal code uses seconds
*/
#ifndef AW_MOISTURESENSOR_MAX_SAMPLINGINTERVAL
	#define AW_MOISTURESENSOR_MAX_SAMPLINGINTERVAL (99*60)
#endif

/*
Sensor sampling interval in seconds while in the calibration menu. Needs to be an integer number
*/
#ifndef AW_MOISTURESENSOR_CALIBRATION_SAMPLINGINTERVAL
	#define AW_MOISTURESENSOR_CALIBRATION_SAMPLINGINTERVAL 1
#endif

/*
Maximum acceptable value for standard deviation.
Anything above is considered too random and means the sensor is probably not connected
*/
#ifndef AW_MOISTURESENSOR_ACCEPTABLE_STANDARD_DEVIATION
	#define AW_MOISTURESENSOR_ACCEPTABLE_STANDARD_DEVIATION 10
#endif

/*
When a sensor is connected, but not getting power, it will consistently report very low values
Any value below this, and we consider that the sensor is not getting power
*/
#ifndef AW_MOISTURESENSOR_ACCEPTABLE_MIN_VALUE
	#define AW_MOISTURESENSOR_ACCEPTABLE_MIN_VALUE 100
#endif

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                               WATER PUMP COMPONENT OPTIONS
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
How many motors can be active at one given time
This is to control the peak power usage, depending on what power supply it is being used
*/
#ifndef AW_MAX_SIMULTANEOUS_PUMPS
	#define AW_MAX_SIMULTANEOUS_PUMPS 3
#endif

/*
Maximum allowed value for water shots (in seconds). Needs to be an integer number
*/
#ifndef AW_SHOT_MAX_DURATION
	#define AW_SHOT_MAX_DURATION 99
#endif

/*
One-off shot duration in seconds. Need to be an integer number
*/
#ifndef AW_SHOT_DEFAULT_DURATION
	#define AW_SHOT_DEFAULT_DURATION 5
#endif
#if AW_SHOT_DEFAULT_DURATION>AW_SHOT_MAX_DURATION
	#error Invalid values
#endif

/*
Minimum time required to pass (in seconds) before a group turns the motor ON again.
This is useful so a group doesn't keep giving motor shots before the sensor reacts properly. It forces a minimum wait
before automated shots.
This does NOT affect manual shots.
*/
#ifndef AW_MINIMUM_TIME_BETWEEN_MOTOR_ON
	#define AW_MINIMUM_TIME_BETWEEN_MOTOR_ON (AW_SHOT_DEFAULT_DURATION*2.0f)
#endif

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                               GRAPHICAL UI COMPONENT OPTIONS
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
If set to 1, it will enable the GraphicalUI component.
At the time of writing, only 320x240 SPI, (ILI9341 + XPT2046) screens are supported
This is the one I'm using:
https://www.amazon.co.uk/gp/product/B07QJW73M3/ref=ppx_yo_dt_b_search_asin_title?ie=UTF8&psc=1
*/
#ifndef AW_GRAPHICALUI_ENABLED
	#define AW_GRAPHICALUI_ENABLED 0
#endif
