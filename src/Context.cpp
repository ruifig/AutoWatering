#include "Context.h"
#include "Utils.h"
#include <Arduino.h>

namespace cz
{

int ProgramData::SoilMoistureSensorData::calcCurrentPercentage() const
{
	return map(currentValue, airValue, waterValue, 0, 100);
}

const ProgramData::SoilMoistureSensorData& ProgramData::getSoilMoistureSensor(unsigned int index)
{
	CZ_ASSERT(index < NumSensors);
	return m_soilMoistureSensorData[index];
}

}  // namespace cz
