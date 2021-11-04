#pragma once

#include <Arduino.h>
#include "crazygaze/micromuc/czmicromuc.h"
#include <MCUFRIEND_kbv.h>

namespace cz
{

enum class PinLocation : uint8_t
{
	Arduino,
	IOExpander,
	Multiplexer
};

template<typename TType, PinLocation Dummy>
struct TPinType
{
	explicit TPinType(uint8_t pin) : raw(pin) {}
	TPinType(const TPinType& other) : raw(other.raw) {}
	TPinType(TPinType&& other) : raw(other.raw) {}
	TPinType& operator=(const TPinType& other)
	{
		raw = other.raw;
		return *this;
	}

	TType raw;
};

using ArduinoPin = TPinType<uint8_t, PinLocation::Arduino>;
using IOExpanderPin = TPinType<uint8_t, PinLocation::IOExpander>;
using MultiplexerPin = TPinType<uint8_t, PinLocation::Multiplexer>;

}

// FASTER_ITERATION
#ifndef FASTER_ITERATION
	#define FASTER_ITERATION 0
#endif

#ifndef MOCK_COMPONENTS
	#define MOCK_COMPONENTS 1
#endif

/**
 * What pin to as CS/SS for the SD card reader
 */
#define SD_CARD_SS_PIN 53

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
 * What i2c address to use for the io expander (0x21..0x27)
 * Note: Don't use address 0x20, because it's used by LCD shield display
 * Note. Internally, 0x20
 */
#define IO_EXPANDER_ADDR 0x21

/**
 * What arduino pin we are using as the IO expander's CS pin
 */
#define ARDUINO_IO_EXPANDER_CS_PIN cz::ArduinoPin(53)

/**
 * What arduino pin we are using to communicate with the multiplexer.
 * Also known as the multiplexer Z pin
 */
#define ARDUINO_MULTIPLEXER_ZPIN cz::ArduinoPin(A15)

/**
 * Pins of the IO expander to use as the s0..s3 pins for the multiplexer
 */
#define IO_EXPANDER_TO_MULTIPLEXER_S0 cz::IOExpanderPin(0)
#define IO_EXPANDER_TO_MULTIPLEXER_S1 cz::IOExpanderPin(1)
#define IO_EXPANDER_TO_MULTIPLEXER_S2 cz::IOExpanderPin(2)
#define IO_EXPANDER_TO_MULTIPLEXER_S3 cz::IOExpanderPin(3)

#define IO_EXPANDER_VPIN_SENSOR_0 cz::IOExpanderPin(4)
#define IO_EXPANDER_VPIN_SENSOR_1 cz::IOExpanderPin(5)
#define IO_EXPANDER_VPIN_SENSOR_2 cz::IOExpanderPin(6)
#define IO_EXPANDER_VPIN_SENSOR_3 cz::IOExpanderPin(7)

#define IO_EXPANDER_MOTOR_0_INPUT1 cz::IOExpanderPin(8)
#define IO_EXPANDER_MOTOR_0_INPUT2 cz::IOExpanderPin(9)
#define IO_EXPANDER_MOTOR_1_INPUT1 cz::IOExpanderPin(10)
#define IO_EXPANDER_MOTOR_1_INPUT2 cz::IOExpanderPin(11)
#define IO_EXPANDER_MOTOR_2_INPUT1 cz::IOExpanderPin(12)
#define IO_EXPANDER_MOTOR_2_INPUT2 cz::IOExpanderPin(13)
#define IO_EXPANDER_MOTOR_3_INPUT1 cz::IOExpanderPin(14)
#define IO_EXPANDER_MOTOR_3_INPUT2 cz::IOExpanderPin(15)

#define MULTIPLEXER_MOISTURE_SENSOR_0 cz::MultiplexerPin(0)
#define MULTIPLEXER_MOISTURE_SENSOR_1 cz::MultiplexerPin(1)
#define MULTIPLEXER_MOISTURE_SENSOR_2 cz::MultiplexerPin(2)
#define MULTIPLEXER_MOISTURE_SENSOR_3 cz::MultiplexerPin(3)

/**
 * How many sensors to support
 */
#define NUM_MOISTURESENSORS 4

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
	#define MOISTURESENSOR_DEFAULT_SAMPLINGINTERVAL 1.0f
	//#define MOISTURESENSOR_DEFAULT_SAMPLINGINTERVAL 1.0f
#else
	#define MOISTURESENSOR_DEFAULT_SAMPLINGINTERVAL 60.0f
#endif

#define DEFAULT_SHOT_DURATION 5.0f

/**
 * Minimum time required to pass (in seconds) before a group turns the motor ON again.
 * This is useful so a group doesn't keep giving motor shots before the sensor reacts properly. It forces a minimum wait
 * before automated shots.
 * This does NOT affect manual shots.
 */
#define MINIMUM_TIME_BETWEEN_MOTOR_ON (DEFAULT_SHOT_DURATION*2.0f)

/**
 * Time in seconds for the LCD's backlight to be turned if no buttons are pressed
 */

#define DEFAULT_LCD_BACKLIGHT_TIMEOUT 10.0f

/**
 * Time in seconds for the LCD to turn off if no buttons are pressed
 */
#define DEFAULT_LCD_IDLE_TIMEOUT 20.0f

/**
 * Time in seconds for the progam to revert to normal operating mode if no buttons are press
 * This is so that if for example the user leaves the system in the menu, after a while it will revert to running normally
 */
#define DEFAULT_REVERT_TO_NORMAL_TIMEOUT 60.0f

/**
 * How long to show the intro for when powering up
 */

 #if FASTER_ITERATION
	#define INTRO_DURATION 0.1f
#else
	#define INTRO_DURATION 1.0f
#endif

#define SCREEN_BKG_COLOUR Colour_Black

#define INTRO_TEXT_COLOUR Colour_Green

#define GRAPH_NUMPOINTS (320-32-2)
#define GRAPH_VALUES_TEXT_COLOUR Colour_DarkGrey
#define GRAPH_VALUES_BKG_COLOUR Colour_Black
#define GRAPH_BORDER_COLOUR Colour_VeryDarkGrey
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
static_assert(1<<5 < GRAPH_HEIGHT, "Reduce number of bits, or increase graph height");
#define GRAPH_POINT_MAXVAL ((1<<GRAPH_POINT_NUM_BITS) - 1)


