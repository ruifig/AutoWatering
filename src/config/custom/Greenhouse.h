#pragma once

#include "../PreConfig.h"

#define AW_SETUP_GREENHOUSE 1
#define CZ_LOG_ENABLED 1
#define AW_COMMAND_CONSOLE_ENABLED 1
#define AW_SCREEN_OFF_TIMEOUT 120
#define AW_MQTT_PUBLISHINTERVAL 1.25f

#define MAX_NUM_I2C_BOARDS 2

#define AW_WIFI_ENABLED 1
#define AW_MQTTUI_ENABLED 1

#define AW_TOUCHUI_ENABLED 1

/**
 * How many sensor/motor pairs to support
 * Each i2c board has 6 pairs
 */
//#define AW_MAX_NUM_PAIRS (6*MAX_NUM_I2C_BOARDS)
//#define AW_MAX_NUM_PAIRS 4
#define AW_MAX_NUM_PAIRS 2

#ifdef __cplusplus
	namespace cz
	{
		class Setup* createSetupObject_GreenHouse();
		inline class Setup* createSetupObject()
		{
			return createSetupObject_GreenHouse();
		}

	} // namespace cz
#endif

#include "../PostConfig.h"
