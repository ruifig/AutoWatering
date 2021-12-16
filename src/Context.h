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

	// We set space for GRAPH_NUMPOINT+1 to make it easier to deal with the display scrolling.
	// Considering there was just 1 update to the queue since the last draw, we can do the following:
	// The first point to draw is index 1, and to draw index 1, we erase the pixel in that pixel with the info from index 0
	using HistoryQueue = TStaticFixedCapacityQueue<GraphPoint, GRAPH_NUMPOINTS + 1>;

	// Data that should be saved/loaded
	struct GroupConfig
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

		void setSensorValue(unsigned int currentValue_)
		{
			numReadings++;
			currentValue = currentValue_;

			if (currentValue > airValue)
			{
				airValue = currentValue;
			}
			else if (currentValue < waterValue)
			{
				waterValue = currentValue;
			}
		}

	};

	class GroupData
	{
	  public:
		GroupData()
		{
		}
		
		void begin(uint8_t index);

		void setMoistureSensorValues(unsigned int currentValue);

		void setMotorState(bool state);
		bool isMotorOn() const;

		// Turns this group on or off
		// On : It will monitor and water
		// Off : Stops monitoring and watering
		void setRunning(bool state);

		bool isRunning() const
		{
			return m_cfg.running;
		}

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

		uint8_t getIndex() const
		{
			return m_index;
		}

		void resetHistory();

	protected:
		friend class ProgramData;
		void save(EEPtr& dst, bool saveConfig, bool saveHistory) const;
		void load(EEPtr& src, bool loadConfig, bool loadHistory);
		int getConfigSize() const
		{
			return sizeof(m_cfg);
		}

	  private:

		// How many seconds to wait between samplings
		uint8_t m_index;

		// Data that should be saved/loaded
		GroupConfig m_cfg;

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
	
	// Saves just 1 single group's config (and not the history
	void saveGroupConfig(uint8_t index);
	void load();

	void begin();

	//
	// Returns true if a group is selected, false otherwise
	bool hasGroupSelected() const;

	//
	// Returns the selected group
	// nullptr if no group is selected
	GroupData* getSelectedGroup();

	// Tries to sets the selected group
	// Depending on the state of the program, it might fail, such as if we are in menus
	// -1 means no group selected
	bool trySetSelectedGroup(int8_t index);

	// This should be called when entering and exiting menus
	void setInMenu(bool inMenu);
	
  private:
	GroupData m_group[NUM_MOISTURESENSORS];
	bool m_moistureSensorMutex = false;
	bool m_inMenu = false;
	int8_t m_selectedGroup = -1;
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

	// Used as a temporary config data when configuring a group
	GroupConfig settingsDummy;
};

extern Context gCtx;

} // namespace cz

