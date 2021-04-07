#pragma once

#include "Config.h"
#include "MCP23017Wrapper.h"
#include "Mux16Channels.h"

namespace cz
{

class ProgramData
{
public:

    struct MoistureSensorData
    {
        // How many seconds to wait between samplings
        float samplingIntervalSeconds = MOISTURESENSOR_DEFAULT_SAMPLINGINTERVAL;
        int airValue = 513;
        int waterValue = 512;
        int currentValue = 0;
        //! Value below which irrigation should be turned on
        int threshold = 0;
        //! Returns the current value in 0..100 format (aka: Percentage)
        int calcCurrentPercentage() const;
    };

    const MoistureSensorData& getMoistureSensor(uint8_t index);
    void setMoistureSensorValues(uint8_t index, int currentValue, int airValue, int waterValue);

    // We only allow 1 sensor to be active at one give time, so we use this as a kind of mutex
    bool tryAcquireMoistureSensorMutex();
    void releaseMoistureSensorMutex();

#if LOG_ENABLED
	void logMoistureSensors();
#else
	void logMoistureSensors() {}
#endif

  private:
    MoistureSensorData m_moistureSensorData[NUM_MOISTURESENSORS];
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
