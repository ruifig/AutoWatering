#pragma once

#include <Arduino.h>

namespace cz
{

enum class PinLocation : uint8_t
{
	MCU,
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

using MCUPin = TPinType<uint8_t, PinLocation::MCU>;
using IOExpanderPin = TPinType<uint8_t, PinLocation::IOExpander>;
using MultiplexerPin = TPinType<uint8_t, PinLocation::Multiplexer>;

}

inline void pinMode(cz::MCUPin pin, PinMode mode)
{
	::pinMode(pin.raw, mode);
}

inline void digitalWrite(cz::MCUPin pin, PinStatus status)
{
	::digitalWrite(pin.raw, status);
}

inline void analogWrite(cz::MCUPin pin, int val)
{
	::analogWrite(pin.raw, val);
}

class DigitalOutputPin
{
  public:
	virtual void write(PinStatus status) = 0;
};

/**
 * Pin used for analog readings
 * The enable/disable methods allow implementation of pin types that have shared resources.
 * For example if an single MCU analog pin is connected to several muxes, enable() can be used activate that mux, and disable used to disable it so the other muxes can be used.
*/
class AnalogInputPin
{
  public:

	/**
	 * This is called before performing readings.
	 * Once a call to enable() is done, any number of calls to read() can be performed.
	 */
	virtual void enable() = 0;

	/**
	 * Called to read an analog value from the pin
	*/
	virtual int read() = 0;

	/**
	 * This is called when done using the pin. No further calls to read() are done until another call to enable() happens.
	*/
	virtual void disable() = 0;
};

class MCUDigitalOutputPin : public DigitalOutputPin
{
  public:
	MCUDigitalOutputPin(const MCUDigitalOutputPin&) = delete;
	MCUDigitalOutputPin& operator=(const MCUDigitalOutputPin&) = delete;
	explicit MCUDigitalOutputPin(uint8_t pin)
		: m_pin(pin)
	{
		::pinMode(m_pin, PinMode::OUTPUT);
	}

	//
	// DigitalOutputPin interface
	//
	virtual void write(PinStatus status) override
	{
		::digitalWrite(m_pin, status);
	}

  private:
	uint8_t m_pin;
};


//
// Digital pin that doesn't do anything
// Useful when an object expects a digital pin that is used to control power, but the board being used is always providing power.
// E.g: For a sensor board controlled by an MCP23017, depending on the design a given sensor might always be on (directly connect to power), or
// to save power it might be only powered on when performing a reading. For this, one of the MCP23017 pins can be used, since it can provide sufficient power.
//
class DummyDigitalPin : public DigitalOutputPin
{
  public:
	virtual void write(PinStatus status) override { }
};

