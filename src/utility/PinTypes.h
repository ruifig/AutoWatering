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
