#pragma once

#include "crazygaze/micromuc/czmicromuc.h"

namespace cz
{

struct Event
{
	enum Type
	{
		SoilMoistureSensorReading,
		StartGroup,
		StopGroup
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

struct StartGroupEvent : public Event
{
	StartGroupEvent(uint8_t index)
		: Event(Event::StartGroup)
		, index(index)
	{}
	
	uint8_t index;
};

struct StopGroupEvent : public Event
{
	StopGroupEvent(uint8_t index)
		: Event(Event::StopGroup)
		, index(index)
	{}
	
	uint8_t index;
};

} // namespace cz


