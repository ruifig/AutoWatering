#include "Context.h"
#include "Utils.h"
#include "crazygaze/micromuc/Logging.h"
#include "crazygaze/micromuc/StringUtils.h"
#include "Component.h"
#include <Arduino.h>

namespace cz
{

Context gCtx;

void updateEEPROM(EEPtr& dst, const uint8_t* src, unsigned int size)
{
	while(size--)
	{
		(*dst).update(*src);
		++dst;
		++src;
	}
}

void readEEPROM(EEPtr& src, uint8_t* dst, unsigned int size)
{
	while(size--)
	{
		*dst = *src; 
		++dst;
		++src;
	}
}

template<typename T, typename = std::enable_if_t<
	std::is_arithmetic_v<T> ||
	std::is_same_v<T, GraphPoint>
	> >
void save(EEPtr& dst, const T& v)
{
	updateEEPROM(dst, reinterpret_cast<const uint8_t*>(&v), sizeof(v));
}

template<typename T, typename = std::enable_if_t<
	std::is_arithmetic_v<T> ||
	std::is_same_v<T, GraphPoint>
	> >
void load(EEPtr& src, T& v)
{
	readEEPROM(src, reinterpret_cast<uint8_t*>(&v), sizeof(v));
}


template<typename T>
void save(EEPtr& src, const TFixedCapacityQueue<T>& v)
{
	int size = v.size();
	save(src, size);
	for(int idx=0; idx<size; idx++)
	{
		const T& t = v.getAtIndex(idx);
		save(src, t);
	}
}

template<typename T>
void load(EEPtr& src, TFixedCapacityQueue<T>& v)
{
	v.clear();
	int size;
	load(src, size);
	CZ_ASSERT(size<=v.capacity());
	while(size--)
	{
		T t;
		load(src, t);
		v.push(t);
	}
}

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

	m_cfg.running = (index==0 || index==1) ? false : true;

#endif
}

void GroupData::setMoistureSensorValues(unsigned int currentValue)
{
	m_cfg.numReadings++;
	m_cfg.currentValue = currentValue;

	if (currentValue > m_cfg.airValue)
	{
		m_cfg.airValue = currentValue;
	}
	else if (currentValue < m_cfg.waterValue)
	{
		m_cfg.waterValue = currentValue;
	}

	// Add to history
	GraphPoint point = {0, 0};
	point.val = map(m_cfg.currentValue, m_cfg.airValue, m_cfg.waterValue, 0, GRAPH_POINT_MAXVAL);

	// Since a motor can be turned on then off without a sensor reading in between, we use
	// m_pendingMotorPoint as a reminder there was a motor event, and so we'll draw that motor plot
	// on the next sensor reading
	point.on = m_motorIsOn || m_pendingMotorPoint;
	m_pendingMotorPoint = false;
	if (m_history.isFull())
	{
		m_history.pop();
	}
	m_history.push(point);

	Component::raiseEvent(SoilMoistureSensorReadingEvent(m_index));
}

void GroupData::setMotorState(bool state)
{
	m_motorIsOn = state;

	// Note that we never set m_pendingMotorPoint to false here, otherwise there would be some motor events
	// that would not be drawn
	if (state)
	{
		m_pendingMotorPoint = true;
	}

	Component::raiseEvent(MotorEvent(m_index, m_motorIsOn));
}

void GroupData::setRunning(bool state)
{
	if (m_cfg.running == state)
	{
		return;
	}

	m_cfg.running = state;
	Component::raiseEvent(GroupOnOffEvent(m_index, state));
}

void GroupData::resetHistory()
{
	m_history.clear();
}

void GroupData::save(EEPtr& dst) const
{
	updateEEPROM(dst, reinterpret_cast<const uint8_t*>(&m_cfg), sizeof(m_cfg));
	cz::save(dst, m_history);
}

void GroupData::load(EEPtr& src)
{
	readEEPROM(src, reinterpret_cast<uint8_t*>(&m_cfg), sizeof(m_cfg));
	cz::load(src, m_history);
}

void ProgramData::begin()
{
	uint8_t idx = 0;
	for(GroupData& g : m_group)
	{
		g.begin(idx);
		idx++;
	}
}

int8_t ProgramData::getSelectedGroup() const
{
	return selectedGroup;
}

void ProgramData::setSelectedGroup(int8_t index)
{
	int8_t previousIndex = selectedGroup;
	selectedGroup = index;
	Component::raiseEvent(GroupSelectedEvent(index, previousIndex));
}

GroupData& ProgramData::getGroupData(uint8_t index)
{
	CZ_ASSERT(index < NUM_MOISTURESENSORS);
	return m_group[index];
}

void ProgramData::save() const
{
	unsigned long startTime = micros();
	EEPtr ptr = EEPROM.begin();

	for(const GroupData& g : m_group)
	{
		g.save(ptr);
	}
	
	unsigned long elapsedMs = (micros() - startTime) / 1000;
	CZ_LOG(logDefault, Log, F("Saving %u bytes to EEPROM took %u ms"), ptr.index, elapsedMs);
	Component::raiseEvent(ConfigSaveEvent());
}

void ProgramData::load()
{
	unsigned long startTime = micros();
	EEPtr ptr = EEPROM.begin();

	for(GroupData& g : m_group)
	{
		g.load(ptr);
	}
	
	unsigned long elapsedMs = (micros() - startTime) / 1000;
	CZ_LOG(logDefault, Log, F("Loading %u bytes from EEPROM took %u ms"), ptr.index, elapsedMs);
	Component::raiseEvent(ConfigLoadEvent());
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
