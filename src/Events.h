#pragma once

#include "crazygaze/micromuc/czmicromuc.h"

namespace cz
{

struct Event
{
	enum Type
	{
		SoilMoistureSensorReading
	};

	Event(Type type) : type(type) {}

	Type type;
};


struct SoilMoistureSensorReadingEvent : public Event
{
	SoilMoistureSensorReadingEvent(uint8_t index)
		: Event(Event::SoilMoistureSensorReading)
		, index(index) {}

	uint8_t index;
};

} // namespace cz


