#include "Context.h"
#include "Utils.h"
#include "crazygaze/micromuc/Logging.h"
#include "crazygaze/micromuc/StringUtils.h"
#include <Arduino.h>

namespace cz
{

void Context::begin()
{
	static_assert(IO_EXPANDER_ADDR>=0x21 && IO_EXPANDER_ADDR<=0x27, "Wrong macro value");
	ioExpander.begin(IO_EXPANDER_ADDR);
	mux.begin();
}

void GroupData::setMoistureSensorValues(int currentValue, int airValue, int waterValue)
{
	m_numReadings++;
	m_currentValue = currentValue;
	m_airValue = airValue;
	m_waterValue = waterValue;
	m_currentPercentageValue = map(m_currentValue, m_airValue, m_waterValue, 0, 100);

	// Add to history
	GraphPoint point;
	point.val = m_currentPercentageValue;
	if (m_history.isFull())
	{
		m_history.pop();
	}
	m_history.push(point);
	m_updated = true;
}

void GroupData::resetHistory()
{
	m_history.clear();
	m_updated = true;
}

GroupData& ProgramData::getGroupData(uint8_t index)
{
	CZ_ASSERT(index < NUM_MOISTURESENSORS);
	return m_group[index];
}


void ProgramData::logMoistureSensors()
{
	char buf[256];
	buf[0] = 0;

	bool changed = false;
	for(auto&& g : m_group)
	{
		if (g.hasChanged())
		{
			g.resetChanged();
			changed = true;
		}
	}

	if (!changed)
	{
		return;
	}

	for(int idx = 0; idx < NUM_MOISTURESENSORS; idx++)
	{
		GroupData& g = m_group[idx];
		if (idx != 0)
		{
			strCatPrintf(buf, " ");
		}
		strCatPrintf(buf, F("(%d:%3u:%3d->%3d, %3d=%3d%%)"), (int)idx, g.getNumReadings(), (int)g.getAirValue(), (int)g.getWaterValue(), (int)g.getCurrentValue(), g.getPercentageValue());

	}

	CZ_LOG(logDefault, Log, F("%s"), buf);
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
