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
		Group,
		Motor,
		
		// Only used for mocking components
		SetMockSensorValue
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

struct GroupEvent : public Event
{
	GroupEvent(uint8_t index, bool started)
		: Event(Event::Group)
		, index(index)
		, started(started)
	{}

	virtual void log() const override
	{
		CZ_LOG(logDefault, Log, F("GroupEvent(%d, %s)"), (int)index, started ? "started" : "stopped");
	}
	
	uint8_t index;
	bool started;
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

struct SetMockSensorValueEvent : public Event
{
	SetMockSensorValueEvent(uint8_t index, int value)
		: Event(Event::SetMockSensorValue)
		, index(index)
		, value(value)
	{
	}

	virtual void log() const override
	{
		CZ_LOG(logDefault, Log, F("SetMockSensorValueEvent(%d, %d)"), (int)index, value);
	}
	
	uint8_t index;
	int value;
};

} // namespace cz


