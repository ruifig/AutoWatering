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

#if CZ_DEBUG
	#define FASTER_ITERATION 1
#else
	#define FASTER_ITERATION 1
#endif

// FASTER_ITERATION
#ifndef FASTER_ITERATION
	#define FASTER_ITERATION 0
#endif

/**
 * What pin to as CS/SS for the SD card reader
 */
#define SD_CARD_SS_PIN 53

/**
 * If 1, it will log to a file in the sd card
 */
#if CZ_LOG_ENABLED
	#define SD_CARD_LOGGING 1
#else
	#define SD_CARD_LOGGING 0
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
 * What arduino pin we are using to communicate with the mutiplexer.
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
	#define MOISTURESENSOR_POWERUP_WAIT 0.20f
#else
	#define MOISTURESENSOR_POWERUP_WAIT 0.20f
#endif

/**
 * Default sensor sampling interval in seconds
 */
#if FASTER_ITERATION
	#define MOISTURESENSOR_DEFAULT_SAMPLINGINTERVAL 1.0f
#else
	#define MOISTURESENSOR_DEFAULT_SAMPLINGINTERVAL 60.0f
#endif

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
	#define INTRO_DURATION 1.0f
#else
	#define INTRO_DURATION 1.0f
#endif

#define GRAPH_NUMPOINTS 200
#define GRAPH_MOTOR_ON_COLOUR TFT_YELLOW
#define GRAPH_MOTOR_OFF_COLOUR TFT_BLACK
#define GRAPH_MOISTURE_OK_COLOUR TFT_GREEN
#define GRAPH_MOISTURE_LOW_COLOUR TFT_RED


//
// * Top line is used for the motor on/off info
// * Rest of the lines are for the moisture level
#define GRAPH_HEIGHT (1+32)


