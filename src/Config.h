#pragma once

#include <Arduino.h>
#include "crazygaze/micromuc/czmicromuc.h"
#include "utility/PinTypes.h"

// FASTER_ITERATION
#ifndef FASTER_ITERATION
	#define FASTER_ITERATION 1
#endif

#ifndef MOCK_COMPONENTS
	#define MOCK_COMPONENTS 1
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
#define MCU_TO_MUX_ZPIN cz::MCUPin(17)

/**
 * Pins of the IO expander to use to set the s0..s2 pins of the mux
 */
#define IO_EXPANDER_TO_MUX_S0 cz::IOExpanderPin(0)
#define IO_EXPANDER_TO_MUX_S1 cz::IOExpanderPin(1)
#define IO_EXPANDER_TO_MUX_S2 cz::IOExpanderPin(2)

/**
 * Pin of the IO Expander used to power the temperature/humidity sensor
 */
#define IO_EXPANDER_VPIN_TEMPSENSOR cz::IOExpanderPin(8+0)

/**
 * Pins of the IO Expander used to power the capacitive soil moisture sensors
 */
#define IO_EXPANDER_VPIN_SENSOR0 cz::IOExpanderPin(8+6)
#define IO_EXPANDER_VPIN_SENSOR1 cz::IOExpanderPin(8+5)
#define IO_EXPANDER_VPIN_SENSOR2 cz::IOExpanderPin(8+4)
#define IO_EXPANDER_VPIN_SENSOR3 cz::IOExpanderPin(8+3)
#define IO_EXPANDER_VPIN_SENSOR4 cz::IOExpanderPin(8+2)
#define IO_EXPANDER_VPIN_SENSOR5 cz::IOExpanderPin(8+1)

/**
 * Pins of the IO Expander used to turn on/off the motors
 */
#define IO_EXPANDER_MOTOR0 cz::IOExpanderPin(8+7)
#define IO_EXPANDER_MOTOR1 cz::IOExpanderPin(0+0)
#define IO_EXPANDER_MOTOR2 cz::IOExpanderPin(0+1)
#define IO_EXPANDER_MOTOR3 cz::IOExpanderPin(0+2)
#define IO_EXPANDER_MOTOR4 cz::IOExpanderPin(0+3)
#define IO_EXPANDER_MOTOR5 cz::IOExpanderPin(0+4)

/**
 * Multiplexer pin used to read temperature/humidity
 */
#define MUX_TEMP_SENSOR cz::MultiplexerPin(3) // Y3

/**
 * Multiplexer pins used to read the soil moisture sensors
 */
#define MUX_MOISTURE_SENSOR0 cz::MultiplexerPin(1) // Y1
#define MUX_MOISTURE_SENSOR1 cz::MultiplexerPin(0) // ...
#define MUX_MOISTURE_SENSOR2 cz::MultiplexerPin(7)
#define MUX_MOISTURE_SENSOR3 cz::MultiplexerPin(5)
#define MUX_MOISTURE_SENSOR4 cz::MultiplexerPin(6)
#define MUX_MOISTURE_SENSOR5 cz::MultiplexerPin(4)

/**
 * How many sensor/motor pairs to support
 */
#define NUM_PAIRS 4

/**
 * When we want to take a moisture reading, we enable power to the the sensor and need to wait a bit before doing the
 * actual reading.
 * This specifies how many seconds to wait before doing the reading
 */
#if FASTER_ITERATION
	#define MOISTURESENSOR_POWERUP_WAIT 0.10f
#else
	#define MOISTURESENSOR_POWERUP_WAIT 0.20f
#endif

/**
 * Default sensor sampling interval in seconds
 */
#if FASTER_ITERATION
	//#define MOISTURESENSOR_DEFAULT_SAMPLINGINTERVAL 5.0f
	#define MOISTURESENSOR_DEFAULT_SAMPLINGINTERVAL 1.0f
#else
	#define MOISTURESENSOR_DEFAULT_SAMPLINGINTERVAL 60.0f
#endif

#define MOISTURESENSOR_CALIBRATION_SAMPLINGINTERVAL 1.0f

#define DEFAULT_SHOT_DURATION 5.0f

/**
 * Minimum time required to pass (in seconds) before a group turns the motor ON again.
 * This is useful so a group doesn't keep giving motor shots before the sensor reacts properly. It forces a minimum wait
 * before automated shots.
 * This does NOT affect manual shots.
 */
#define MINIMUM_TIME_BETWEEN_MOTOR_ON (DEFAULT_SHOT_DURATION*2.0f)

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
	#define INTRO_DURATION 0.1f
#else
	#define INTRO_DURATION 1.0f
#endif

/**
 * Default screen: 0..100.
 * Actual screen classes scale this accordingly to whatever range they use 
 */
#define SCREEN_DEFAULT_BRIGHTNESS 100

#define SCREEN_BKG_COLOUR Colour_Black

#define INTRO_TEXT_COLOUR Colour_Green

#define GRAPH_NUMPOINTS (320-32-2)
#define GRAPH_VALUES_TEXT_COLOUR Colour_DarkGrey
#define GRAPH_VALUES_BKG_COLOUR Colour_Black
#define GRAPH_BORDER_COLOUR Colour_DarkGrey
#define GRAPH_SELECTED_BORDER_COLOUR Colour_Green
#define GRAPH_NOTSELECTED_BORDER_COLOUR Colour_Black
#define GRAPH_BKG_COLOUR Colour_Black
#define GRAPH_NOTRUNNING_TEXT_COLOUR Colour_Red
#define GRAPH_MOTOR_ON_COLOUR Colour_Yellow
#define GRAPH_MOTOR_OFF_COLOUR Colour_Black
#define GRAPH_MOISTURELEVEL_COLOUR Colour_Red


//
// * Top line is used for the motor on/off info
// * Rest of the lines are for the moisture level
#define GRAPH_HEIGHT (1+32)
#define GRAPH_POINT_NUM_BITS 5
static_assert(1<<GRAPH_POINT_NUM_BITS < GRAPH_HEIGHT, "Reduce number of bits, or increase graph height");
#define GRAPH_POINT_MAXVAL ((1<<GRAPH_POINT_NUM_BITS) - 1)


