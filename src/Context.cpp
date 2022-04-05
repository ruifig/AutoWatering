#include "Context.h"
#include "crazygaze/micromuc/Logging.h"
#include "crazygaze/micromuc/StringUtils.h"
#include "Component.h"
#include <Arduino.h>
#include <type_traits>


auto test()
{
	std::chrono::seconds secs;
	return secs;
}

namespace cz
{

Context gCtx;

void updateEEPROM(AT24C::Ptr& dst, const uint8_t* src, unsigned int size)
{
	while(size--)
	{
		(*dst).update(*src);
		++dst;
		++src;
	}
}

void readEEPROM(AT24C::Ptr& src, uint8_t* dst, unsigned int size)
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
void save(AT24C::Ptr& dst, const T& v)
{
	updateEEPROM(dst, reinterpret_cast<const uint8_t*>(&v), sizeof(v));
}

template<typename T, typename = std::enable_if_t<
	std::is_arithmetic_v<T> ||
	std::is_same_v<T, GraphPoint>
	> >
void load(AT24C::Ptr& src, T& v)
{
	readEEPROM(src, reinterpret_cast<uint8_t*>(&v), sizeof(v));
}

template<typename T>
void save(AT24C::Ptr& src, const TFixedCapacityQueue<T>& v)
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
void load(AT24C::Ptr& src, TFixedCapacityQueue<T>& v)
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
	static_assert(IO_EXPANDER_ADDR>=0x0 && IO_EXPANDER_ADDR<=0x7, "Wrong macro value");
	ioExpander.begin(IO_EXPANDER_ADDR);
	mux.begin();
	data.begin();
}

///////////////////////////////////////////////////////////////////////
// GroupData
///////////////////////////////////////////////////////////////////////

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

	m_cfg.running = index==2;

#endif
}

void GroupData::setMoistureSensorValues(unsigned int currentValue, bool isCalibrating)
{
	m_cfg.setSensorValue(currentValue, isCalibrating);

	// Add to history if this his a real value (as in, we are not calibrating this sensor)
	if (!isCalibrating)
	{
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
	}

	Component::raiseEvent(SoilMoistureSensorReadingEvent(m_index, isCalibrating));
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

bool GroupData::isMotorOn() const
{
	return m_motorIsOn;
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

void GroupData::startCalibration()
{
	m_calibrating = true;
	Component::raiseEvent(SensorCalibrationEvent(m_index, true));
}

void GroupData::stopCalibration()
{
	m_calibrating = false;
	Component::raiseEvent(SensorCalibrationEvent(m_index, false));
}

void GroupData::resetHistory()
{
	m_history.clear();
}

void GroupData::save(AT24C::Ptr& dst, bool saveConfig, bool saveHistory) const
{

	if (saveConfig)
	{
		CZ_LOG(logDefault, Log, F("Saving group %d config at address %u"), m_index, dst.getAddress());
		updateEEPROM(dst, reinterpret_cast<const uint8_t*>(&m_cfg), sizeof(m_cfg));
	}

	if (saveHistory)
	{
		CZ_LOG(logDefault, Log, F("Saving group %d history at address %u"), m_index, dst.getAddress());
		cz::save(dst, m_history);
	}
}

void GroupData::load(AT24C::Ptr& src, bool loadConfig, bool loadHistory)
{
	if (loadConfig)
	{
		CZ_LOG(logDefault, Log, F("Loading group %d config from address %u"), m_index, src.getAddress());
		readEEPROM(src, reinterpret_cast<uint8_t*>(&m_cfg), sizeof(m_cfg));
	}

	if (loadHistory)
	{
		CZ_LOG(logDefault, Log, F("Loading group %d history from address %u"), m_index, src.getAddress());
		cz::load(src, m_history);
	}
}

///////////////////////////////////////////////////////////////////////
// ProgramData
///////////////////////////////////////////////////////////////////////

ProgramData::ProgramData(Context& outer)
	: m_outer(outer)
{
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

GroupData* ProgramData::getSelectedGroup()
{
	return m_selectedGroup==-1 ? nullptr : &m_group[m_selectedGroup];
}

bool ProgramData::hasGroupSelected() const
{
	return m_selectedGroup == -1 ? false : true;
}

bool ProgramData::trySetSelectedGroup(int8_t index)
{
	if (m_inGroupConfigMenu)
	{
		return false;
	}

	if (index != m_selectedGroup)
	{
		int8_t previousIndex = m_selectedGroup;
		m_selectedGroup = index;
		Component::raiseEvent(GroupSelectedEvent(index, previousIndex));
	}
	return true;
}

void ProgramData::setInGroupConfigMenu(bool inMenu)
{
	m_inGroupConfigMenu = inMenu;
}

GroupData& ProgramData::getGroupData(uint8_t index)
{
	CZ_ASSERT(index < NUM_PAIRS);
	return m_group[index];
}

void ProgramData::save() const
{
	unsigned long startTime = micros();
	AT24C::Ptr ptr = m_outer.eeprom.at(0);

	// We save the configs first because they are fixed size, and so we can load/save groups individually when coming
	// out of the configuration menu
	for(const GroupData& g : m_group)
	{
		g.save(ptr, true, false);
	}

	for(const GroupData& g : m_group)
	{
		g.save(ptr, false, true);
	}
	
	unsigned long elapsedMs = (micros() - startTime) / 1000;
	CZ_LOG(logDefault, Log, F("Saving %u bytes to EEPROM took %u ms"), ptr.getAddress(), elapsedMs);
	Component::raiseEvent(ConfigSaveEvent());
}

void ProgramData::saveGroupConfig(uint8_t index)
{
	unsigned long startTime = micros();
	AT24C::Ptr ptr = m_outer.eeprom.at(0);

	for(uint8_t idx = 0; idx<index; ++idx)
	{
		ptr += m_group[idx].getConfigSize();
	}

	m_group[index].save(ptr, true, false);
	unsigned long elapsedMs = (micros() - startTime) / 1000;
	CZ_LOG(logDefault, Log, F("Saving %u bytes to EEPROM took %u ms"), ptr.getAddress(), elapsedMs);
	Component::raiseEvent(ConfigLoadEvent());
}

void ProgramData::load()
{
	unsigned long startTime = micros();
	AT24C::Ptr ptr = m_outer.eeprom.at(0);

	for(GroupData& g : m_group)
	{
		g.load(ptr, true, false);
	}

	for(GroupData& g : m_group)
	{
		g.load(ptr, false, true);
	}
	
	unsigned long elapsedMs = (micros() - startTime) / 1000;
	CZ_LOG(logDefault, Log, F("Loading %u bytes from EEPROM took %u ms"), ptr.getAddress(), elapsedMs);
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

} // namespace cz
