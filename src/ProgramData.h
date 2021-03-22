#pragma once

#include "Config.h"

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

} // namespace cz
