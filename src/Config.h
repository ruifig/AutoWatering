#pragma once

#include <Arduino.h>
#include "crazygaze/micromuc/czmicromuc.h"
#include "utility/PinTypes.h"

// FASTER_ITERATION
#ifndef FASTER_ITERATION
	#define FASTER_ITERATION 1
#endif

#ifndef MOCK_COMPONENTS
	#define MOCK_COMPONENTS 0
#endif

/**
 * What pin to as CS/SS for the SD card reader
 */
//#define SD_CARD_SS_PIN 53

/**
 * If 1, it will log to a file in the sd card
 */
#if CZ_LOG_ENABLED
	#define SD_CARD_LOGGING 0
#else
	#define SD_CARD_LOGGING 0
#endif

#ifndef CONSOLE_COMMANDS
	#if CZ_DEBUG
		#define CONSOLE_COMMANDS 1
	#else
		#define CONSOLE_COMMANDS 0
	#endif
#endif


/**
 * Display pins
 */
#define TFT_PIN_DC cz::MCUPin(9)
#define TFT_PIN_CS cz::MCUPin(10)
#define TFT_PIN_BACKLIGHT cz::MCUPin(4)

/**
 * Touch controller pins
 */
#define TOUCH_PIN_CS cz::MCUPin(2)
#define TOUCH_PIN_IRQ cz::MCUPin(3)

/**
 * Note. Internally it adds 0x20, which is the base address
 */
#define IO_EXPANDER_ADDR 0x0

/**
 * What arduino pin we are using to communicate with the multiplexer.
 * Also known as the multiplexer Z pin
 * This needs to be an analog capable pin, so we can do sensor readings
 */
#define MCU_TO_MUX_ZPIN cz::MCUPin(16)

/**
 * Pins of the IO expander to use to set the s0..s2 pins of the mux
 */
#define IO_EXPANDER_TO_MUX_S0 cz::IOExpanderPin(0+0)
#define IO_EXPANDER_TO_MUX_S1 cz::IOExpanderPin(0+1)
#define IO_EXPANDER_TO_MUX_S2 cz::IOExpanderPin(0+2)

/**
 * Pins of the IO Expander used to turn on/off the motors
 */
#define IO_EXPANDER_MOTOR0 cz::IOExpanderPin(0+3)
#define IO_EXPANDER_MOTOR1 cz::IOExpanderPin(0+4)
#define IO_EXPANDER_MOTOR2 cz::IOExpanderPin(0+5)
#define IO_EXPANDER_MOTOR3 cz::IOExpanderPin(0+6)
#define IO_EXPANDER_MOTOR4 cz::IOExpanderPin(0+7)
#define IO_EXPANDER_MOTOR5 cz::IOExpanderPin(8+6)

/**
 * Pins of the IO Expander used to power the capacitive soil moisture sensors
 */
#define IO_EXPANDER_VPIN_SENSOR0 cz::IOExpanderPin(8+5)
#define IO_EXPANDER_VPIN_SENSOR1 cz::IOExpanderPin(8+4)
#define IO_EXPANDER_VPIN_SENSOR2 cz::IOExpanderPin(8+3)
#define IO_EXPANDER_VPIN_SENSOR3 cz::IOExpanderPin(8+2)
#define IO_EXPANDER_VPIN_SENSOR4 cz::IOExpanderPin(8+1)
#define IO_EXPANDER_VPIN_SENSOR5 cz::IOExpanderPin(8+0)

/*
 * Pin from the IO Expander to the Mux's E pin.
 * This allows us to turn off the Mux, so multiple sensor boards can share a single sensor reading pin.
 * LOW - Mux enabled
 * HIGH - Mux disabled
 */
#define IO_EXPANDER_TO_MUX_ENABLE cz::IOExpanderPin(8+7)

/**
 * Multiplexer pins used to read the soil moisture sensors
 */
#define MUX_MOISTURE_SENSOR0 cz::MultiplexerPin(7) // Y0
#define MUX_MOISTURE_SENSOR1 cz::MultiplexerPin(5) // ...
#define MUX_MOISTURE_SENSOR2 cz::MultiplexerPin(3)
#define MUX_MOISTURE_SENSOR3 cz::MultiplexerPin(0)
#define MUX_MOISTURE_SENSOR4 cz::MultiplexerPin(1)
#define MUX_MOISTURE_SENSOR5 cz::MultiplexerPin(2)

#define MAX_NUM_I2C_BOARDS 2

/**
 * How many sensor/motor pairs to support
 * Each i2c board has 6 pairs
 */
#define MAX_NUM_PAIRS (6*MAX_NUM_I2C_BOARDS)

/**
 * How many sensor/motor pairs fit on the screen
 */
#define VISIBLE_NUM_PAIRS 4

/**
 * How many motors can be active at one given time
 * This is to control the peak power usage, depending on what power supply it is being used
 */
#define MAX_SIMULTANEOUS_MOTORS 3

/**
 * How many sensors can be active at one given time
 * This should be always 1, because we are sharing a single Arduino pin for the analog reads
 */
#define MAX_SIMULTANEOUS_SENSORS 1

/**
 * When we want to take a moisture reading, we enable power to the the sensor and need to wait a bit before doing the
 * actual reading.
 * This specifies how many seconds to wait before doing the reading
 */
#if FASTER_ITERATION
	#define MOISTURESENSOR_POWERUP_WAIT 0.20f
#else
	#define MOISTURESENSOR_POWERUP_WAIT 0.20f
#endif

/**
 * Default sensor sampling interval in seconds. Needs to be integer number.
 * Note that internally the sampling interval is tracked in seconds, but the UI shows it in minutes. This is because
 * allowing it in seconds is easier for development since we can have the sensor reacting fast, BUT showing the settings UI in seconds
 * would be cumbersome since the user will probably want big intervals between samplings (intervals in minutes probably)
 */
#if FASTER_ITERATION
	#define MOISTURESENSOR_DEFAULT_SAMPLINGINTERVAL 1
#else
	#define MOISTURESENSOR_DEFAULT_SAMPLINGINTERVAL 60
#endif

/**
 * Maximum sampling interval allowed in seconds (integer number)
 * This also limits the the maximum value the UI will allow and show.
 * Please note that the UI shows sampling intervals in minutes, but the internal code uses
 */
#define MOISTURESENSOR_MAX_SAMPLINGINTERVAL (99*60)

/**
 * Sensor sampling interval while in the calibration menu. Needs to be an integer number
 */
#define MOISTURESENSOR_CALIBRATION_SAMPLINGINTERVAL 1

/**
 * Water shot duration in seconds. Need to be an integer number
 */
#define SHOT_DEFAULT_DURATION 5

/**
 * Maximum allowed value for water shots (in seconds). Needs to be an integer number
 */
#define SHOT_MAX_DURATION 99

// Temperature/humidity sensor sampling interval in seconds.
#if FASTER_ITERATION
	#define TEMPSENSOR_DEFAULT_SAMPLINGINTERVAL 10.0f
#else
	#define TEMPSENSOR_DEFAULT_SAMPLINGINTERVAL 60.0f
#endif

/**
 * Minimum time required to pass (in seconds) before a group turns the motor ON again.
 * This is useful so a group doesn't keep giving motor shots before the sensor reacts properly. It forces a minimum wait
 * before automated shots.
 * This does NOT affect manual shots.
 */
#define MINIMUM_TIME_BETWEEN_MOTOR_ON (SHOT_DEFAULT_DURATION*2.0f)

/**
 * Time in seconds for the program to revert to normal operating mode if no buttons are press
 * This is so that if for example the user leaves the system in the menu, after a while it will revert to running normally
 */
// #TODO : This is not being used yet
#define DEFAULT_REVERT_TO_NORMAL_TIMEOUT 60.0f

/**
 * How long to show the intro for when powering up
 */
#if FASTER_ITERATION
	#define INTRO_DURATION 0.25f
#else
	#define INTRO_DURATION 1.0f
#endif


/**
 * How long the boot menu should stay up before defaulting to "Load"
 */
#define BOOTMENU_COUNTDOWN 2.0f


/**
 * Default screen: 0..100.
 * Actual screen classes scale this accordingly to whatever range they use 
 */
#define SCREEN_DEFAULT_BRIGHTNESS 100

/**
 * How long to wait until turning off the screen backlight, if there are no touch events detected
 * If 0, screen timeout is considered disabled (as-in, screen is always on)
 */
#define SCREEN_OFF_TIMEOUT 0

/**
 * How fast to dim the screen to 0.
 * This is in percentiles per second. E.g: a value of 10 means the brightness will go down at 10% per second
 */
#define SCREEN_OFF_DIM_SPEED 10

#define SCREEN_BKG_COLOUR Colour_Black

#define INTRO_TEXT_COLOUR Colour_Green

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240

// Width in pixels for the group number label (shown left of the group history plot)
#define GROUP_NUM_WIDTH 16
#define GRAPH_NUMPOINTS (SCREEN_WIDTH-GROUP_NUM_WIDTH-32-2)
#define GRAPH_VALUES_TEXT_COLOUR Colour_LightGrey
#define GRAPH_VALUES_BKG_COLOUR Colour_Black
#define GRAPH_BORDER_COLOUR Colour_LightGrey
#define GRAPH_SELECTED_BORDER_COLOUR Colour_Green
#define GRAPH_NOTSELECTED_BORDER_COLOUR Colour_Black
#define GRAPH_BKG_COLOUR Colour_Black
#define GRAPH_NOTRUNNING_TEXT_COLOUR Colour_Red
#define GRAPH_MOTOR_ON_COLOUR Colour_Yellow
#define GRAPH_MOTOR_OFF_COLOUR Colour_Black
#define GRAPH_MOISTURELEVEL_COLOUR Colour_Cyan
#define GRAPH_MOISTURELEVEL_ERROR_COLOUR Colour_Red
#define GRAPH_THRESHOLDMARKER_COLOUR Colour(0x52ab)
#define TEMPERATURE_LABEL_TEXT_COLOUR Colour_Yellow
#define HUMIDITY_LABEL_TEXT_COLOUR Colour_Cyan
#define RUNNINGTIME_LABEL_TEXT_COLOUR Colour_Yellow


//
// * Top line is used for the motor on/off info
// * Rest of the lines are for the moisture level
#define GRAPH_HEIGHT (1+32)
#define GRAPH_POINT_NUM_BITS 5
static_assert(1<<GRAPH_POINT_NUM_BITS < GRAPH_HEIGHT, "Reduce number of bits, or increase graph height");
#define GRAPH_POINT_MAXVAL ((1<<GRAPH_POINT_NUM_BITS) - 1)

// Maximum acceptable value for standard deviation.
// Anything above is considered too random and means the sensor is probably not connected
#define MOISTURESENSOR_ACCEPTABLE_STANDARD_DEVIATION 10

// When a sensor is connected, but not getting power, it will consistently report very low values
// Any value below this, and we consider that the sensor is not getting power
#define MOISTURESENSOR_ACCEPTABLE_MIN_VALUE 100
