#pragma once

#include "Config.h"
#include "MCP23017Wrapper.h"
#include "Mux16Channels.h"
#include "Utils.h"
#include <crazygaze/micromuc/Queue.h>

namespace cz
{
	struct GraphPoint
	{
		// 0..100 moisture level
		unsigned int val : 7;
		// Tells if the motor was on at this point
		bool on : 1;
	};

	static_assert(sizeof(GraphPoint)==1, "GraphPoint size must be 1");

	using HistoryQueue = TStaticFixedCapacityQueue<GraphPoint, GRAPH_NUMPOINTS>;

    class GroupData
    {
	  public:
		void setMoistureSensorValues(int currentValue, int airValue, int waterValue);

		unsigned int getAirValue() const
		{
			return m_airValue;
		}

		unsigned int getWaterValue() const
		{
			return m_waterValue;
		}

		unsigned int getCurrentValue() const
		{
			return m_currentValue;
		}

		unsigned int getPercentageValue() const
		{
			return m_currentPercentageValue;
		}

		unsigned int getPercentageThreshold() const
		{
			return m_thresholdPercentage;
		}

		float getSamplingInterval() const
		{
			return m_samplingInterval;
		}

		bool hasChanged() const
		{
			return m_updated;
		}

		void resetChanged()
		{
			m_updated = false;
		}

		const HistoryQueue& getHistory()
		{
			return m_history;
		}

		unsigned int getNumReadings() const
		{
			return m_numReadings;
		}


		void resetHistory();

	  private:
        // How many seconds to wait between samplings
        float m_samplingInterval = MOISTURESENSOR_DEFAULT_SAMPLINGINTERVAL;
		int m_numReadings = 0;
        unsigned int m_airValue = 513;
        unsigned int m_waterValue = 512;
        unsigned int m_currentValue = 0;
		uint8_t m_currentPercentageValue = 0;
        //! Value below which irrigation should be turned on
        int m_threshold = 0;
		// #TODO : Remove or implement this properly
        int m_thresholdPercentage = 50;
        //! Returns the current value in 0..100 format (aka: Percentage)
        int calcCurrentPercentage() const;
		HistoryQueue m_history;
		bool m_updated = false;
    };

class ProgramData
{
public:
    GroupData& getGroupData(uint8_t index);

    // We only allow 1 sensor to be active at one give time, so we use this as a kind of mutex
    bool tryAcquireMoistureSensorMutex();
    void releaseMoistureSensorMutex();

	void logMoistureSensors();

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

    MCP23017Wrapper ioExpander;
    Mux16Channels mux;
    ProgramData data;
};

} // namespace cz
