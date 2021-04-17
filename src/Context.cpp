#include "Context.h"
#include "Utils.h"
#include <Arduino.h>

namespace cz
{

void Context::begin()
{
	static_assert(IO_EXPANDER_ADDR>=0x21 && IO_EXPANDER_ADDR<=0x27, "Wrong macro value");
	ioExpander.begin(IO_EXPANDER_ADDR);
	mux.begin();
}

int ProgramData::MoistureSensorData::calcCurrentPercentage() const
{
	return map(currentValue, airValue, waterValue, 0, 100);
}

const ProgramData::MoistureSensorData& ProgramData::getMoistureSensor(uint8_t index)
{
	CZ_ASSERT(index < NUM_MOISTURESENSORS);
	return m_moistureSensorData[index];
}

void ProgramData::setMoistureSensorValues(uint8_t index, int currentValue, int airValue, int waterValue)
{
	CZ_ASSERT(index < NUM_MOISTURESENSORS);
	auto& s = m_moistureSensorData[index];
	s.currentValue = currentValue;
	s.airValue = airValue;
	s.waterValue = waterValue;

#if 0
	if (index==3)
	{
		CZ_LOG_LN("Sensor %d: (%3d->%3d) (%3d=%3d%%)", (int)index, (int)s.airValue, (int)s.waterValue, (int)s.currentValue, s.calcCurrentPercentage());
	}
#endif
}

void ProgramData::logMoistureSensors()
{
	char buf[128] = {};

	for(int idx = 0; idx < NUM_MOISTURESENSORS; idx++)
	{
		auto& s = m_moistureSensorData[idx];
		if (idx != 0)
		{
			strCatPrintf(buf, " ");
		}
		strCatPrintf(buf, "(%d:%3d->%3d, %3d=%3d%%)", (int)idx, (int)s.airValue, (int)s.waterValue, (int)s.currentValue, s.calcCurrentPercentage());
	}

	CZ_LOGLN("%s", buf);
}

bool ProgramData::tryAcquireMoistureSensorMutex()
{
	if (m_moistureSensorMutex)
	{
		return false;
	}
	else
	{
		m_moistureSensorMutex = true;
		return true;
	}
}

void ProgramData::releaseMoistureSensorMutex()
{
	CZ_ASSERT(m_moistureSensorMutex==true);
	m_moistureSensorMutex = false;
}

}  // namespace cz
