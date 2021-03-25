#pragma once

#include "Config.h"
#include "MCP23S17Wrapper.h"
#include "Mux16Channels.h"

namespace cz
{

class ProgramData
{
public:

    struct SoilMoistureSensorData
    {
        // How many seconds to wait between samplings
        int samplingIntervalSeconds = DEFAULT_SENSOR_SAMPLING_INTERVAL;
        int airValue = 513;
        int waterValue = 512;
        int currentValue = 0;
        //! Value below which irrigation should be turned on
        int threshold = 0;
        //! Returns the current value in 0..100 format (aka: Percentage)
        int calcCurrentPercentage() const;
    };

    const SoilMoistureSensorData& getSoilMoistureSensor(unsigned int index);

private:
    static constexpr int NumSensors = 4;
    
    SoilMoistureSensorData m_soilMoistureSensorData[NumSensors];
};

struct Context
{
    Context()
        : ioExpander(&SPI, ARDUINO_IO_EXPANDER_CS_PIN, 0)
        , mux(
            ioExpander,
            IO_EXPANDER_TO_MULTIPLEXER_S0,
            IO_EXPANDER_TO_MULTIPLEXER_S1,
            IO_EXPANDER_TO_MULTIPLEXER_S2,
            IO_EXPANDER_TO_MULTIPLEXER_S3,
            ARDUINO_MULTIPLEXER_ZPIN)
    {
    }

    MCP23S17Wrapper ioExpander;
    Mux16Channels mux;
    ProgramData data;
};

} // namespace cz
