#include "ProgramData.h"
#include <Arduino.h>

int ProgramData::MoistureSensorData::calcCurrentPercentage() const
{
    return map(currentValue, airValue, waterValue, 0, 100);
}
