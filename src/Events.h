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
		SensorCalibration,
		SoilMoistureSensorReading,
		SoilMoistureSensorThresholdUpdate,
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

// #RVF : Remove this is not used
struct SensorCalibrationEvent : public Event
{
	SensorCalibrationEvent(int8_t group, bool start)
		: Event(Event::SensorCalibration)
		, group(group)
		, start(start)
	{
	}

	virtual void log() const override
	{
		CZ_LOG(logEvents, Log, F("SensorCalibrationEvent(%s)"), start ? "true" : "false");
	}

	int8_t group;
	// Calibration started or stopped
	bool start;
};

struct SoilMoistureSensorReadingEvent : public Event
{
	SoilMoistureSensorReadingEvent(uint8_t index, bool calibrating, const SensorReading& reading)
		: Event(Event::SoilMoistureSensorReading)
		, index(index)
		, calibrating(calibrating)
		, reading(reading)
	{
	}

	virtual void log() const override
	{
		CZ_LOG(logEvents, Verbose, F("SoilMoistureSensorReadingEvent(%d)"), (int)index);
	}

	uint8_t index;
	// If this is true, this was a reading done while calibrating, and some components might want to ignore it
	bool calibrating;
	SensorReading reading;
};

struct SoilMoistureSensorThresholdUpdateEvent : public Event
{
	SoilMoistureSensorThresholdUpdateEvent(uint8_t index)
		: Event(Event::SoilMoistureSensorThresholdUpdate)
		, index(index)
	{
	}

	virtual void log() const override
	{
		CZ_LOG(logEvents, Verbose, F("SoilMoistureSensorThresholdUpdateEvent(%d)"), (int)index);
	}

	uint8_t index;
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
		CZ_LOG(logEvents, Verbose, F("TemperatureSensorReadingEvent(%2.1fC)"), temperatureC);
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
		CZ_LOG(logEvents, Verbose, F("HumiditySensorReadingEvent(%2.1f%%)"), humidity);
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

