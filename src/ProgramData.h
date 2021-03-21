#pragma once

#include <assert.h>

class ProgramData
{
public:

    struct MoistureSensorData
    {
        // How many seconds to wait between samplings
        int samplingIntervalSeconds = 5;
        int airValue = 513;
        int waterValue = 512;
        int currentValue = 0;
        //! Value below which irrigation should be turned on
        int threshold = 0;
        //! Returns the current value in 0..100 format (aka: Percentage)
        int calcCurrentPercentage() const;
    };

    const MoistureSensorData& getMoistureSensor(unsigned int index)
    {
        assert(index < NumSensors);
        return m_moistureSensorData[index];
    }

private:
    static constexpr int NumSensors = 4;
    
    MoistureSensorData m_moistureSensorData[NumSensors];
};
