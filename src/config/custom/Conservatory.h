#pragma once

/*

Overview of this board:

* No screen. Uses MQTT only
* Up to 8 sensor/motor pairs
* Temperature/Humidity sensor (HTU21D sensor module)
* It uses mosfets to turn the motors individidually
* It uses 1 single mosfet to turn on all the moisture sensors
* No discreet eeprom IC. It uses Arduino-Pico's eeprom emulation (https://arduino-pico.readthedocs.io/en/latest/eeprom.html)
	* Be careful with how many times you write the config, since the rp2040's flash doesn't allow as many writes as a real eeprom IC (e.g: AT24C256)
*/

#include "../PreConfig.h"

#define AW_SETUP_CONSERVATORY 1
#define CZ_LOG_ENABLED 1
#define AW_COMMAND_CONSOLE_ENABLED 1


// No need for graphical UI
#define AW_TOUCHUI_ENABLED 0
#define AW_WIFI_ENABLED 1
#define AW_MQTTUI_ENABLED 1
#define AW_MQTT_PUBLISHINTERVAL 1.25f

/**
 * How many sensor/motor pairs to support
 * Each i2c board has 6 pairs
 */
#define AW_MAX_NUM_PAIRS 8

#ifdef __cplusplus
	namespace cz
	{
		class Setup* createSetupObject_Conservatory();
		inline class Setup* createSetupObject()
		{
			return createSetupObject_Conservatory();
		}

	} // namespace cz
#endif

#include "../PostConfig.h"
