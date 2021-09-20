#include "Context.h"
#include "Utils.h"
#include "crazygaze/micromuc/Logging.h"
#include "crazygaze/micromuc/StringUtils.h"
#include "Component.h"
#include <Arduino.h>

namespace cz
{

void Context::begin()
{
	static_assert(IO_EXPANDER_ADDR>=0x21 && IO_EXPANDER_ADDR<=0x27, "Wrong macro value");
	ioExpander.begin(IO_EXPANDER_ADDR);
	mux.begin();

	data.begin();

}


void GroupData::begin(uint8_t index)
{
	m_index = index;

// Fill the history with some values, for testing purposes
#if FASTER_ITERATION
	for(int i=0; i<20; i++)
	{
		m_history.push({GRAPH_POINT_MAXVAL, false});
	}

	for(int i=0; i<20; i++)
	{
		m_history.push({0, false});
	}

	for(int i=0; i<20; i++)
	{
		m_history.push({GRAPH_POINT_MAXVAL/4, true});
	}

	for(int i=0; i<20; i++)
	{
		m_history.push({GRAPH_POINT_MAXVAL/2 -1, false});
	}

	for(int i=0; i<20; i++)
	{
		m_history.push({GRAPH_POINT_MAXVAL/2, true});
	}

	for(int i=0; i<20; i++)
	{
		m_history.push({(GRAPH_POINT_MAXVAL*3)/4, false});
	}

	for(int i=0; i<20; i++)
	{
		m_history.push({GRAPH_POINT_MAXVAL, true});
	}

	while(!m_history.isFull())
	{
		m_history.push({GRAPH_POINT_MAXVAL/3, false});
	}

	m_updateCount = GRAPH_NUMPOINTS; 

#endif
}

void GroupData::setMoistureSensorValues(int currentValue, int airValue, int waterValue)
{
	m_numReadings++;
	m_currentValue = currentValue;
	m_airValue = airValue;
	m_waterValue = waterValue;
	m_currentPercentageValue = map(m_currentValue, m_airValue, m_waterValue, 0, 100);

	// Add to history
	GraphPoint point = {0, 0};
	point.val = map(m_currentValue, m_airValue, m_waterValue, 0, GRAPH_POINT_MAXVAL);

	// Since a motor can be turned on then off without a sensor reading in between, we use
	// m_pendingMotorDrawing as a reminder there was a motor event, and so we'll draw that motor plot
	// on the next sensor reading
	point.on = m_motorIsOn || m_pendingMotorDrawing;
	m_pendingMotorDrawing = false;
	if (m_history.isFull())
	{
		m_history.pop();
	}
	m_history.push(point);
	m_updateCount++;

	Component::raiseEvent(SoilMoistureSensorReadingEvent(m_index));
}

void GroupData::setMotorState(bool state)
{
	m_motorIsOn = state;

	// Note that we never set m_pendingMotorDrawing to false here, otherwise there would be some motor events
	// that would not be drawn
	if (state)
	{
		m_pendingMotorDrawing = true;
	}

	Component::raiseEvent(MotorEvent(m_index, m_motorIsOn));
}

void GroupData::start()
{
	if (m_running)
	{
		return;
	}

	m_running = true;
	Component::raiseEvent(StartGroupEvent(m_index));
}

void GroupData::stop()
{
	if (!m_running)
	{
		return;
	}

	m_running = false;
	Component::raiseEvent(StopGroupEvent(m_index));
}

void GroupData::resetHistory()
{
	m_history.clear();
	m_updateCount = 0;
}


void ProgramData::begin()
{
	uint8_t idx = 0;
	for(auto&& g : m_group)
	{
		g.begin(idx);
		idx++;
	}
}

GroupData& ProgramData::getGroupData(uint8_t index)
{
	CZ_ASSERT(index < NUM_MOISTURESENSORS);
	return m_group[index];
}


#if 0
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

#endif

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
