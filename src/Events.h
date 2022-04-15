#pragma once

#include "crazygaze/micromuc/czmicromuc.h"
#include "crazygaze/micromuc/Logging.h"
#include "Context.h"

CZ_DECLARE_LOG_CATEGORY(logEvents, Log, Verbose)

namespace cz
{

struct Event
{
	enum Type
	{
		ConfigLoad,
		ConfigSave,
		SoilMoistureSensorReading,
		SoilMoistureSensorCalibrationReading,
		TemperatureSensorReading,
		HumiditySensorReading,
		GroupOnOff,
		GroupSelected,
		Motor,
		
		// Only used for mocking components
		SetMockSensorValue,
		SetMockSensorErrorStatus
	};

	Event(Type type) : type(type) {}
	virtual void log() const = 0;

	Type type;
};

struct ConfigLoadEvent : public Event
{
	ConfigLoadEvent(int8_t group = -1)
		: Event(Event::ConfigLoad)
		, group(group)
	{
	}
	
	virtual void log() const override
	{
		CZ_LOG(logEvents, Log, F("ConfigLoadEvent"));
	}

	// if -1, then its loading the entire config.
	// if !=-1 then its loading this specific group only
	int8_t group;
};

struct ConfigSaveEvent : public Event
{
	ConfigSaveEvent(int8_t group = -1)
		: Event(Event::ConfigSave)
	{
	}
	
	virtual void log() const override
	{
		CZ_LOG(logEvents, Log, F("ConfigSaveEvent"));
	}

	// if -1, then its saving the entire config.
	// if !=-1 then its saving this specific group only
	int8_t group;
};

struct SoilMoistureSensorReadingEvent : public Event
{
	SoilMoistureSensorReadingEvent(uint8_t index, const SensorReading& reading)
		: Event(Event::SoilMoistureSensorReading)
		, index(index)
		, reading(reading)
	{
	}

	virtual void log() const override
	{
		CZ_LOG(logEvents, Log, F("SoilMoistureSensorReadingEvent(%d)"), (int)index);
	}

	uint8_t index;
	SensorReading reading;
};

struct SoilMoistureSensorCalibrationReadingEvent : public Event
{
	SoilMoistureSensorCalibrationReadingEvent(uint8_t index, const SensorReading& reading)
		: Event(Event::SoilMoistureSensorCalibrationReading)
		, index(index)
		, reading(reading)
	{
	}

	virtual void log() const override
	{
		CZ_LOG(logEvents, Log, F("SoilMoistureSensorCalibrationReadingEvent(%d)"), (int)index);
	}

	uint8_t index;
	SensorReading reading;
};

struct TemperatureSensorReadingEvent : public Event
{
	TemperatureSensorReadingEvent(float temperatureC)
		: Event(Event::TemperatureSensorReading)
		, temperatureC(temperatureC)
	{
	}

	virtual void log() const override
	{
		CZ_LOG(logEvents, Log, F("TemperatureSensorReadingEvent(%2.1fC)"), temperatureC);
	}

	// temperature in Celcius
	float temperatureC;
};

struct HumiditySensorReadingEvent : public Event
{
	HumiditySensorReadingEvent(float humidity)
		: Event(Event::HumiditySensorReading)
		, humidity(humidity)
	{
	}

	virtual void log() const override
	{
		CZ_LOG(logEvents, Log, F("HumiditySensorReadingEvent(%2.1f%%)"), humidity);
	}

	// Humidity - 0..100%
	float humidity;
};



struct GroupOnOffEvent : public Event
{
	GroupOnOffEvent(uint8_t index, bool started)
		: Event(Event::GroupOnOff)
		, index(index)
		, started(started)
	{}

	virtual void log() const override
	{
		CZ_LOG(logEvents, Log, F("GroupOnOffEvent(%d, %s)"), (int)index, started ? "started" : "stopped");
	}
	
	uint8_t index;
	bool started;
};

struct GroupSelectedEvent : public Event
{
	GroupSelectedEvent(int8_t index, int8_t previousIndex)
		: Event(Event::GroupSelected)
		, index(index)
		, previousIndex(previousIndex)
	{}

	virtual void log() const override
	{
		CZ_LOG(logEvents, Log, F("GroupSelectedEvent(%d, %d)"), (int)index, (int)previousIndex);
	}
	
	// What group was selected
	// If -1, then there is no group selected
	int8_t index;
	// What group was previously selected, or -1 if there was not group previously selected
	int8_t previousIndex;
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
		CZ_LOG(logEvents, Log, F("MotorEvent(%d, %s)"), (int)index, started ? "started" : "stopped");
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
		CZ_LOG(logEvents, Log, F("SetMockSensorValueEvent(%d, %d)"), (int)index, value);
	}
	
	uint8_t index;
	int value;
};

struct SetMockSensorErrorStatusEvent : public Event
{
	SetMockSensorErrorStatusEvent(uint8_t index, SensorReading::Status status)
		: Event(Event::SetMockSensorErrorStatus)
		, index(index)
		, status(status)
	{
	}

	virtual void log() const override
	{
		CZ_LOG(logEvents, Log, F("SetMockSensorErrorStatusEvent(%d, %d)"), (int)index, status);
	}
	
	uint8_t index;
	SensorReading::Status status;
};

} // namespace cz

