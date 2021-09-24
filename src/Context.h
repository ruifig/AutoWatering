#pragma once

#include "Config.h"
#include "MCP23017Wrapper.h"
#include "Mux16Channels.h"
#include "Utils.h"
#include <crazygaze/micromuc/Queue.h>
#include <EEPROM.h>

namespace cz
{
	struct GraphPoint
	{
		// 0..100 moisture level
		unsigned int val : GRAPH_POINT_NUM_BITS;
		// Tells if the motor was on at this point
		bool on : 1;
	};

	static_assert(sizeof(GraphPoint)==1, "GraphPoint size must be 1");

	using HistoryQueue = TStaticFixedCapacityQueue<GraphPoint, GRAPH_NUMPOINTS>;

	class GroupData
	{
	  public:
		void begin(uint8_t index);

		void setMoistureSensorValues(int currentValue);

		void setMotorState(bool state);

		// Starts running this group (aka: It will monitor and water)
		void start();
		// Stops running this group (aka: Stops monitoring and watering)
		void stop();

		unsigned int getAirValue() const
		{
			return m_cfg.airValue;
		}

		unsigned int getWaterValue() const
		{
			return m_cfg.waterValue;
		}

		unsigned int getCurrentValue() const
		{
			return m_cfg.currentValue;
		}

		unsigned int getPercentageValue() const
		{
			return map(m_cfg.currentValue, m_cfg.airValue, m_cfg.waterValue, 0, 100);
		}

		void setThresholdValue(unsigned int value)
		{
			m_cfg.thresholdValue = value;
		}

		unsigned int getThresholdValue() const
		{
			return m_cfg.thresholdValue;
		}

		unsigned int getPercentageThreshold() const
		{
			return map(m_cfg.thresholdValue, m_cfg.airValue, m_cfg.waterValue, 0, 100);
		}

		float getSamplingInterval() const
		{
			return m_cfg.samplingInterval;
		}

		float getShotDuration() const
		{
			return m_cfg.shotDuration;	
		}

		const HistoryQueue& getHistory()
		{
			return m_history;
		}

		uint32_t getNumReadings() const
		{
			return m_cfg.numReadings;
		}

		void resetHistory();


	protected:
		friend class ProgramData;
		void save(EEPtr& dst) const;
		void load(EEPtr& src);

	  private:

		// How many seconds to wait between samplings
		uint8_t m_index;

		// Data that should be saved/loaded
		struct
		{
			// Tells if this group is currently running
			bool running = false;
			// Sensor sampling interval in seconds
			float samplingInterval = MOISTURESENSOR_DEFAULT_SAMPLINGINTERVAL;
			// Motor shot duration in seconds
			float shotDuration = DEFAULT_SHOT_DURATION;
			// Number of sensor readings done
			uint32_t numReadings = 0;

			// The sensor values decreases as moisture increases. (High Value = Dry, Low Value = Wet)
			// Air and water values are calculated automatically as sensor values are provided. This means the user wipe clean the sensor
			// to set the air value, and then submerse the sensor to set the water value.
			// Having air and water properly is not a requirement for things to work, since what matter is that the system know what 
			// sensor value is the threshold to turn on/off the motor
			#define START_AIR_VALUE 400
			#define START_WATER_VALUE 300
			unsigned int airValue = START_AIR_VALUE;
			unsigned int waterValue = START_WATER_VALUE;
			// Current sensor value
			unsigned int currentValue = START_AIR_VALUE - (START_AIR_VALUE-START_WATER_VALUE)/2;

			// Value above which irrigation should be turned on
			// NOTE: ABOVE because higher values means drier.
			// Using 0 as initial value, which means it will not turn on the motor until things are setup properly
			unsigned int thresholdValue = 0;
		} m_cfg;

		HistoryQueue m_history;

		bool m_motorIsOn = false;
		// Used so we can detect when the motor was turned on and off before a sensor data point is inserted, so we can
		// add the motor flag to the next sensor data point when that happens.
		bool m_pendingMotorPoint = false;
	};

class ProgramData
{
public:
	GroupData& getGroupData(uint8_t index);

	// We only allow 1 sensor to be active at one give time, so we use this as a kind of mutex
	bool tryAcquireMoistureSensorMutex();
	void releaseMoistureSensorMutex();

	void save() const;
	void load();

	void begin();
  private:
	GroupData m_group[NUM_MOISTURESENSORS];
	bool m_moistureSensorMutex = false;
};

struct Context
{
	Context()
		: mux(
			ioExpander,
			IO_EXPANDER_TO_MULTIPLEXER_S0,
			IO_EXPANDER_TO_MULTIPLEXER_S1,
			IO_EXPANDER_TO_MULTIPLEXER_S2,
			IO_EXPANDER_TO_MULTIPLEXER_S3,
			ARDUINO_MULTIPLEXER_ZPIN)
	{
	}

	void begin();

#if MOCK_COMPONENTS
	MockMCP23017Wrapper ioExpander;
#else
	MCP23017Wrapper ioExpander;
#endif
	Mux16Channels mux;
	ProgramData data;
};

} // namespace cz

