#pragma once

#include "crazygaze/micromuc/czmicromuc.h"
#include "crazygaze/micromuc/Logging.h"

namespace cz
{

struct Event
{
	enum Type
	{
		SoilMoistureSensorReading,
		StartGroup,
		StopGroup,
		MotorStarted,
		MotorStopped,
	};

	Event(Type type) : type(type) {}
	virtual void log() const = 0;

	Type type;
};

struct SoilMoistureSensorReadingEvent : public Event
{
	SoilMoistureSensorReadingEvent(uint8_t index)
		: Event(Event::SoilMoistureSensorReading)
		, index(index) {}

	virtual void log() const override
	{
		return;
		CZ_LOG(logDefault, Log, F("SoilMoistureSensorReadingEvent(%d)"), (int)index);
	}

	uint8_t index;
};

struct StartGroupEvent : public Event
{
	StartGroupEvent(uint8_t index)
		: Event(Event::StartGroup)
		, index(index)
	{}

	virtual void log() const override
	{
		CZ_LOG(logDefault, Log, F("StartGroupEvent(%d)"), (int)index);
	}
	
	uint8_t index;
};

struct StopGroupEvent : public Event
{
	StopGroupEvent(uint8_t index)
		: Event(Event::StopGroup)
		, index(index)
	{}

	virtual void log() const override
	{
		CZ_LOG(logDefault, Log, F("StopGroupEvent(%d)"), (int)index);
	}
	
	uint8_t index;
};

struct MotorStarted : public Event
{
	MotorStarted(uint8_t index)
		: Event(Event::MotorStarted)
		, index(index)
	{
	}

	virtual void log() const override
	{
		CZ_LOG(logDefault, Log, F("MotorStarted(%d)"), (int)index);
	}
	
	uint8_t index;
};

struct MotorStopped : public Event
{
	MotorStopped(uint8_t index)
		: Event(Event::MotorStopped)
		, index(index)
	{
	}

	virtual void log() const override
	{
		CZ_LOG(logDefault, Log, F("MotorStopped(%d)"), (int)index);
	}
	
	uint8_t index;
};

} // namespace cz


