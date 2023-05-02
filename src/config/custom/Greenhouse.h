#pragma once

#include "../PreConfig.h"

#define CZ_LOG_ENABLED 1
#define AW_COMMAND_CONSOLE_ENABLED 1
#define AW_SCREEN_OFF_TIMEOUT 0

#define MAX_NUM_I2C_BOARDS 2

/**
 * How many sensor/motor pairs to support
 * Each i2c board has 6 pairs
 */
#define AW_MAX_NUM_PAIRS (6*MAX_NUM_I2C_BOARDS)

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
