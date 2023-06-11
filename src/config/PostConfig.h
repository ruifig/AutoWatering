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
//                               PROFILER OPTIONS
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
If 1, it enables profiling code
*/
#ifndef CZ_PROFILER
	#if AW_DEBUG
		#define CZ_PROFILER 1
	#else
		#define CZ_PROFILER 0
	#endif
#endif

#if CZ_PROFILER && !AW_COMMAND_CONSOLE_ENABLED
	#error CZ_PROFILER needs AW_COMMAND_CONSOLE_ENABLED
#endif

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                               NETWORK OPTIONS
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
 If 0, no wifi is used
 This will disable any features that require a network connection.
 If set to 1, make sure you provide your network details. See Config/Secrets.h for more information
 */
#ifndef AW_WIFI_ENABLED
	#define AW_WIFI_ENABLED 1
#endif

/*
How long to wait between attempts to reconnect to the mqtt broker
*/
#ifndef AW_MQTT_CONNECTION_RETRY_INTERVAL
	#define AW_MQTT_CONNECTION_RETRY_INTERVAL 5.0f
#endif

/*
Interval between publishes.
This limits how fast we can publish values, since Adafruit IO has a strict limit:
* Free account: 30 points per minute
* Paid account: 60 points per minute
*/
#ifndef AW_MQTT_PUBLISHINTERVAL
	#define AW_MQTT_PUBLISHINTERVAL 2.0f
#endif

/**
At the time of writting, I've noticed that sometimes even if Wifi is supposed to be connected, establishing TCP connections fails, and never recovers.
Setting this to 0 means no disconnect/reconnect is done
Setting this to N (where N>0), will perform a wifi disconnect/reconnect after failing to establish the TCP connection N times.

WARNING: If the Watchdog is enabled (see AW_AW_WATCHDOG_ENABLED), then a wifi reconnect attempt will very likely trigger the watchdog.
 */
#ifndef AW_MQTT_WIFI_RECONNECT
	#define AW_MQTT_WIFI_RECONNECT 5
#endif

/*
If 1, sensor readings (e.g: temperature, humidity, and soil moisture), will be published even if their value didn't change
*/
#ifndef AW_MQTT_SENSOR_FORCESYNC
	#define AW_MQTT_SENSOR_FORCESYNC 1
#endif

/*
Minimum time to wait before publishing a new sensor reading.
The touch UI allows setting the soil moisture sensor readings intervals to 0, which means "as fast as possible". This makes testing 
the sensors easier since we can see the values change as fast as possible.
But this presents a possible problem for MQTT. For example, Adafruit IO has restrictions on how fast we can publish.
At the time of writing, a free account, allows 30 publishes per minute, while a paid account allows 60 publishes per minute. 
To make this even worse, those restrictions are PER ACCOUNT, not per device.
So, this macro allows specifying the mininum interval between publishes (per sensor)

Note that this works in conjunction with AQ_MQTT_PUBLISHINTERVAL, which dictates the absolute cap.
*/
#ifndef AW_MQTT_MOISTURESENSOR_MININTERVAL
	#define AW_MQTT_MOISTURESENSOR_MININTERVAL AW_MQTT_PUBLISHINTERVAL
#endif

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
#ifndef AW_THSENSOR_ENABLED
	#define AW_THSENSOR_ENABLED 1
#endif

// Default sampling interval, in seconds
#if AW_FASTER_ITERATION
	#define AW_THSENSOR_SAMPLINGINTERVAL 120.0f
#else
	#define AW_THSENSOR_SAMPLINGINTERVAL 300.0f
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
	#define AW_MAX_SIMULTANEOUS_PUMPS 2
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
//                               MQTT UI COMPONENT OPTIONS
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef AW_MQTTUI_ENABLED
	#define AW_MQTTUI_ENABLED 1
#endif

#if AW_MQTTUI_ENABLED
	#if !AW_WIFI_ENABLED
		#error MQTT requires WIFI
	#endif
#endif

/*
Time in seconds to wait until we trigger a local config save when we receive a change from the MQTT broker
This avoids saving to flash/EEPROM every time we receive an update.
It allows the user to fiddle with the MQTT UI to adjust settings without every single change being saved.
For every change, the delay is reset to this value, and once the delay reaches zero, the saving occurs.
*/
#ifndef AW_MQTTUI_SAVEDELAY
	#define AW_MQTTUI_SAVEDELAY 5.0f
#endif

/*
How long to wait in seconds until the full config is received from the MQTT broker.
This timer starts after Wifi connection is detected.
If the config is not received within this time window, the local config is published
*/
#ifndef AW_MQTTUI_WAITFORCONFIG_TIMEOUT
	#define AW_MQTTUI_WAITFORCONFIG_TIMEOUT 5.0f
#endif

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                               GRAPHICAL UI COMPONENT OPTIONS
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
If set to 1, it will enable the GraphicalUI component.
At the time of writing, only 320x240 SPI, (ILI9341 + XPT2046) screens are supported
This is the one I'm using:
https://www.amazon.co.uk/gp/product/B07QJW73M3/ref=ppx_yo_dt_b_search_asin_title?ie=UTF8&psc=1

Also, disabling this component means no UI and therefore no UI code using the CPU, but it doesn't completely remove
all the UI or TouchController code. That's something that I still need to improve to remove the firmware size.

Some more notes:
	- Disabling this component doesn't complete remove UI and Touch controller code. That code won't run, but the linker somehow still thinks it's needed
		- I should fix that in the future
	- Lots of these macros have carefuly planned values to aligh UI, etc, so changing their values might break things.
*/
#ifndef AW_TOUCHUI_ENABLED
	#define AW_TOUCHUI_ENABLED 1
#endif

// At the moment, the only touch screen supported is SPI based, so we enable use of SPI
#define AW_SPI_ENABLED AW_TOUCHUI_ENABLED

// At the moment these are fixed
#define AW_SCREEN_WIDTH 320
#define AW_SCREEN_HEIGHT 240

//
// TFT_eSPI defines
//
// If this is set, User_Setup_Select.h file will not load the user setting header files
#ifndef USER_SETUP_LOADED
	#define USER_SETUP_LOADED 1
#endif
// Define the TFT driver, pins etc here:
#ifndef ILI9341_DRIVER
	#define ILI9341_DRIVER 1
#endif
#ifndef TFT_MISO
	#define TFT_MISO 4
#endif
#ifndef TFT_SCLK
	#define TFT_SCLK 6
#endif
#ifndef TFT_MOSI
	#define TFT_MOSI 7
#endif
#ifndef TFT_RST
	#define TFT_RST 8
#endif
#ifndef TFT_DC
	#define TFT_DC 9
#endif
#ifndef TFT_BL
	#define TFT_BL 10
#endif
#ifndef TFT_CS
	#define TFT_CS 11
#endif
// TFT_eSPI expects us to set TOUCH_CS if we want it to implement touch screen support.
// But since it doesn't automatically make use of interrupts out of the box, I'm using a separate library that does.
// Therefore, I'm not calling this TOUCH_CS but TOUCH_MY_CS so it doesn't get picked up by TFT_eSPI.
#ifndef TOUCH_MY_CS
	#define TOUCH_MY_CS 12
#endif
#ifndef TOUCH_MY_IRQ
	#define TOUCH_MY_IRQ 13
#endif
//Other settings
#ifndef LOAD_GLCD
	#define LOAD_GLCD 1
#endif
#ifndef LOAD_FONT2
	#define LOAD_FONT2 1
#endif
#ifndef LOAD_FONT4
	#define LOAD_FONT4 1
#endif
#ifndef LOAD_FONT6
	#define LOAD_FONT6 1
#endif
#ifndef LOAD_FONT7
	#define LOAD_FONT7 1
#endif
#ifndef LOAD_FONT8
	#define LOAD_FONT8 1
#endif
#ifndef LOAD_GFXFF
	#define LOAD_GFXFF 1
#endif
#ifndef SMOOTH_FONT
	#define SMOOTH_FONT 1
#endif
#ifndef SPI_FREQUENCY
	#define SPI_FREQUENCY 27000000
#endif


/*
How long to show the intro for when powering up
*/
#ifndef AW_INTRO_DURATION
	#if AW_FASTER_ITERATION
		#define AW_INTRO_DURATION 0.25f
	#else
		#define AW_INTRO_DURATION 1.0f
	#endif
#endif

/*
How long the boot menu should stay up before defaulting to "Load"
*/
#ifndef AW_BOOTMENU_COUNTDOWN
	#define AW_BOOTMENU_COUNTDOWN 2.0f
#endif

/*
Default screen: 0..100.
Actual screen classes scale this accordingly to whatever range they use 
*/
#ifndef AW_SCREEN_DEFAULT_BRIGHTNESS
	#define AW_SCREEN_DEFAULT_BRIGHTNESS 100
#endif

/**
 * How long to wait (in seconds) until turning off the screen backlight, if there are no touch events detected
 * If 0, screen timeout is considered disabled (as-in, screen is always on)
 */
#ifndef AW_SCREEN_OFF_TIMEOUT
	#define AW_SCREEN_OFF_TIMEOUT 30
#endif

/*
How fast to dim the screen to 0.
This is in percentiles per second. E.g: a value of 10 means the brightness will go down at 10% per second
*/
#ifndef AW_SCREEN_OFF_DIM_SPEED
	#define AW_SCREEN_OFF_DIM_SPEED 10
#endif

/*
How many sensor/motor pairs fit on the screen
*/
#ifndef AW_VISIBLE_NUM_PAIRS
	#define AW_VISIBLE_NUM_PAIRS 4
#endif

//
// Some colours
//
#ifndef AW_SCREEN_BKG_COLOUR
	#define AW_SCREEN_BKG_COLOUR Colour_Black
#endif

#ifndef AW_INTRO_TEXT_COLOUR
	#define AW_INTRO_TEXT_COLOUR Colour_Green
#endif

// Width in pixels for the group number label (shown left of the group history plot)
#ifndef AW_GROUP_NUM_WIDTH
	#define AW_GROUP_NUM_WIDTH 16
#endif

#ifndef AW_GRAPH_NUMPOINTS
	#define AW_GRAPH_NUMPOINTS (AW_SCREEN_WIDTH-AW_GROUP_NUM_WIDTH-32-2)
#endif

#ifndef AW_GRAPH_VALUES_TEXT_COLOUR
	#define AW_GRAPH_VALUES_TEXT_COLOUR Colour_LightGrey
#endif

#ifndef AW_GRAPH_VALUES_BKG_COLOUR
	#define AW_GRAPH_VALUES_BKG_COLOUR Colour_Black
#endif

#ifndef AW_GRAPH_BORDER_COLOUR
	#define AW_GRAPH_BORDER_COLOUR Colour_LightGrey
#endif

#ifndef AW_GRAPH_SELECTED_BORDER_COLOUR
	#define AW_GRAPH_SELECTED_BORDER_COLOUR Colour_Green
#endif

#ifndef AW_GRAPH_NOTSELECTED_BORDER_COLOUR
	#define AW_GRAPH_NOTSELECTED_BORDER_COLOUR Colour_Black
#endif

#ifndef AW_GRAPH_BKG_COLOUR
	#define AW_GRAPH_BKG_COLOUR Colour_Black
#endif

#ifndef AW_GRAPH_NOTRUNNING_TEXT_COLOUR
	#define AW_GRAPH_NOTRUNNING_TEXT_COLOUR Colour_Red
#endif

#ifndef AW_GRAPH_MOTOR_ON_COLOUR
	#define AW_GRAPH_MOTOR_ON_COLOUR Colour_Yellow
#endif

#ifndef AW_GRAPH_MOTOR_OFF_COLOUR
	#define AW_GRAPH_MOTOR_OFF_COLOUR Colour_Black
#endif

#ifndef AW_GRAPH_MOISTURELEVEL_COLOUR
	#define AW_GRAPH_MOISTURELEVEL_COLOUR Colour_Cyan
#endif

#ifndef AW_GRAPH_MOISTURELEVEL_ERROR_COLOUR
	#define AW_GRAPH_MOISTURELEVEL_ERROR_COLOUR Colour_Red
#endif

#ifndef AW_GRAPH_THRESHOLDMARKER_COLOUR
	#define AW_GRAPH_THRESHOLDMARKER_COLOUR Colour(0x52ab)
#endif

#ifndef AW_TEMPERATURE_LABEL_TEXT_COLOUR
	#define AW_TEMPERATURE_LABEL_TEXT_COLOUR Colour_Yellow
#endif

#ifndef AW_HUMIDITY_LABEL_TEXT_COLOUR
	#define AW_HUMIDITY_LABEL_TEXT_COLOUR Colour_Cyan
#endif

#ifndef AW_RUNNINGTIME_LABEL_TEXT_COLOUR
	#define AW_RUNNINGTIME_LABEL_TEXT_COLOUR Colour_Yellow
#endif

#ifndef AW_BATTERYLEVEL_LABEL_TEXT_COLOUR
	#define AW_BATTERYLEVEL_LABEL_TEXT_COLOUR Colour_White
#endif

//
// * Top line is used for the motor on/off info
// * Rest of the lines are for the moisture level
#ifndef AW_GRAPH_HEIGHT
	#define AW_GRAPH_HEIGHT (1+32)
#endif

// These is actually a data format option, and not really an UI option
// It's recommend you don't change this
#ifndef AW_GRAPH_POINT_NUM_BITS
	#define AW_GRAPH_POINT_NUM_BITS 5
#endif
// This file is included for every single C++ or C file, and static_assert is a C++ only feature
#ifdef __cplusplus 
	static_assert((1<<AW_GRAPH_POINT_NUM_BITS) < AW_GRAPH_HEIGHT, "Reduce number of bits, or increase graph height");
#endif
#define AW_GRAPH_POINT_MAXVAL ((1<<AW_GRAPH_POINT_NUM_BITS) - 1)

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                               I2C OPTIONS
//
// I2C support should be enabled if any components or the board design makes use of i2c
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef AW_I2C_ENABLED
	#if AW_THSENSOR_ENABLED || AW_STORAGE_AT24C_ENABLED
		#define AW_I2C_ENABLED 1
	#else
		#define AW_I2C_ENABLED 0
	#endif
#endif

#if AW_I2C_ENABLED

	// SDA pin to use
	#ifndef AW_I2C_SDAPIN
		#define AW_I2C_SDAPIN 0
	#endif

	// SCL pin to use
	#ifndef AW_I2C_SCLPIN
		#define AW_I2C_SCLPIN 1
	#endif

	// Speed (in hz) to set i2c to.
	// This default value was picked to support the combination of components I developed the project with. E.g:
	//		MCP23017 : 400kHz at 3.3v
	//		AT24Cxxx : 400kHz at 2.7v, 2.5v
	#ifndef AW_I2C_SPEEDHZ
		#define AW_I2C_SPEEDHZ 400000
	#endif
#endif


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                               Make sure the user config defined mandatory things
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// This needs to be defined by the user config
// It specifies how many sensor+motor pairs the setup supports
#ifndef AW_MAX_NUM_PAIRS
	#error AW_MAX_NUM_PAIRS must be defined by the user config
#endif

#define AW_CONFIG_DONE
#include "Secrets.h"

//
// IMPORTANT:
// The user config header must typedef "Setup" to the actual setup class
#ifdef __cplusplus 
namespace cz
{
	class SoilMoistureSensor;
	class PumpMonitor;

	/*
	A custom setup must implement this interface
	*/
	class Setup
	{
	  public:

		virtual void begin() = 0;

		/*
		 * When booting a fresh Autowatering device that was never configured, it will use this as the feed group when connecting
		 * to Adafruit IO.
		 * 
		 * The first thing you need to do for a new device is to go to the Adafruit IO and add a new entry to the "/feeds/aw-unnamed.name"
		 * feed with the new device name.
		 * The device will pick up the change, save the name to the EEPROM and reboot. Once it boots and you see the new feed group was
		 * created, you can delete the "aw-unnamed" group.
		 * 
		 * If you don't plan to build more than 1 device, you can just override this in your custom setup to return the actual name you
		 * want to use
		 * 
		 * WARNING: The name should not be longer than AW_DEVICENAME_MAX_LEN-1
		 */
		virtual const char* getDefaultName() const
		{
			return "aw-unnamed";
		}

		// Called for each sensor+motor pair
		virtual SoilMoistureSensor* createSoilMoistureSensor(int index) = 0;
		virtual PumpMonitor* createPumpMonitor(int index) = 0;

		// These are called by setup() after calling gSetup->begin()
		void createSoilMoistureSensors();
		void createPumpMonitors();

		SoilMoistureSensor* getSoilMoistureSensor(int index)
		{
			return m_soilMoistureSensors[index];
		}

		PumpMonitor* getPumpMonitor(int index)
		{
			return m_pumpMonitors[index];
		}

	  protected:
		SoilMoistureSensor* m_soilMoistureSensors[AW_MAX_NUM_PAIRS];
		PumpMonitor* m_pumpMonitors[AW_MAX_NUM_PAIRS];
	};

	extern Setup* gSetup;
}
#endif
