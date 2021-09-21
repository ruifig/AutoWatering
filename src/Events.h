#pragma once

#include "crazygaze/micromuc/czmicromuc.h"
#include "crazygaze/micromuc/Logging.h"

namespace cz
{

struct Event
{
	enum Type
	{
		ConfigLoad,
		ConfigSave,
		SoilMoistureSensorReading,
		StartGroup,
		StopGroup,
		Motor
	};

	Event(Type type) : type(type) {}
	virtual void log() const = 0;

	Type type;
};

struct ConfigLoadEvent : public Event
{
	ConfigLoadEvent()
		: Event(Event::ConfigLoad)
	{
	}
	
	virtual void log() const override
	{
		CZ_LOG(logDefault, Log, F("ConfigLoadEvent"));
	}
};

struct ConfigSaveEvent : public Event
{
	ConfigSaveEvent()
		: Event(Event::ConfigSave)
	{
	}
	
	virtual void log() const override
	{
		CZ_LOG(logDefault, Log, F("ConfigSaveEvent"));
	}
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

struct MotorEvent : public Event
{
	MotorEvent(uint8_t index, bool started)
		: Event(Event::Motor)
		, index(index)
		, started(started)
	{
	}

	virtual void log() const override
	{
		CZ_LOG(logDefault, Log, F("MotorEvent(%d, %s)"), (int)index, started ? "started" : "stopped");
	}
	
	uint8_t index;
	bool started;
};

} // namespace cz


