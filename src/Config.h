#pragma once

#include <Arduino.h>

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

#define MULTIPLEXER_MOISTURE_SENSOR_0 cz::MultiplexerPin(0)
#define MULTIPLEXER_MOISTURE_SENSOR_1 cz::MultiplexerPin(1)
#define MULTIPLEXER_MOISTURE_SENSOR_2 cz::MultiplexerPin(2)
#define MULTIPLEXER_MOISTURE_SENSOR_3 cz::MultiplexerPin(3)

/**
 * How many sensors to support
 */
#define NUM_SENSORS 4

/**
 * Default sensor sampling interval in seconds
 */
#define DEFAULT_SENSOR_SAMPLING_INTERVAL 5

/**
 * Time in seconds for the LCD's backlight to be turned if no buttons are pressed
 */

#define DEFAULT_LCD_BACKLIGHT_TIMEOUT 10 

/**
 * Time in seconds for the LCD to turn off if no buttons are pressed
 */
#define DEFAULT_LCD_IDLE_TIMEOUT 20

/**
 * Time in seconds for the progam to revert to normal operating mode if no buttons are press
 * This is so that if for example the user leaves the system in the menu, after a while it will revert to running normally
 */
#define DEFAULT_REVERT_TO_NORMAL_TIMEOUT 60


/**
 * How long to show the intro for when powering up
 */
#define INTRO_DURATION 5
