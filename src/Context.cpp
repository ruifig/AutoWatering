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
		//(*dst) = *src;
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
// GroupConfig
///////////////////////////////////////////////////////////////////////

void GroupConfig::log() const
{
	CZ_LOG(logDefault, Log, F("    m_data.running=%u"), (unsigned int)m_data.running);
	CZ_LOG(logDefault, Log, F("    m_data.samplingInterval=%u"), (unsigned int)m_data.samplingInterval);
	CZ_LOG(logDefault, Log, F("    m_data.shotDuration=%u"), (unsigned int)m_data.shotDuration);
	CZ_LOG(logDefault, Log, F("    m_data.waterValue=%u"), (unsigned int)m_data.waterValue);
	CZ_LOG(logDefault, Log, F("    m_data.airValue=%u"), (unsigned int)m_data.airValue);
	CZ_LOG(logDefault, Log, F("    m_data.thresholdValue=%u, %u%%"), (unsigned int)m_data.thresholdValue, getThresholdValueAsPercentage());
	CZ_LOG(logDefault, Log, F("    m_isDirty=%u"), (unsigned int)m_isDirty);
	CZ_LOG(logDefault, Log, F("    m_currentValue=%u"), (unsigned int)m_currentValue);
	CZ_LOG(logDefault, Log, F("    m_numReadings=%u"), (unsigned int)m_numReadings);
}

void GroupConfig::save(AT24C::Ptr& dst) const
{
	if (m_isDirty)
	{
		m_isDirty = false;
		updateEEPROM(dst, reinterpret_cast<const uint8_t*>(&m_data), sizeof(m_data));
	}
	else
	{
		CZ_LOG(logDefault, Log, F("Group is not dirty. Nothing to save"));
		dst += sizeof(m_data);
	}
}

void GroupConfig::load(AT24C::Ptr& src)
{
	readEEPROM(src, reinterpret_cast<uint8_t*>(&m_data), sizeof(m_data));
	m_isDirty = false;
}

bool GroupConfig::isDirty() const
{
	return m_isDirty;
}

bool GroupConfig::isRunning() const
{
	return m_data.running;
}

void GroupConfig::setRunning(bool running)
{
	if (running != m_data.running)
	{
		m_isDirty = true;
		m_data.running = running;
	}
}

unsigned int GroupConfig::getThresholdValue() const
{
	return m_data.thresholdValue;
}

unsigned int GroupConfig::getThresholdValueAsPercentage() const
{
	unsigned int tmp = cz::clamp(m_data.thresholdValue, m_data.waterValue, m_data.airValue);
	return map(tmp, m_data.airValue, m_data.waterValue, 0, 100);
}

void GroupConfig::setThresholdValue(unsigned int value)
{
	if (value != m_data.thresholdValue)
	{
		m_isDirty = true;
		m_data.thresholdValue = value;
	}
}

void GroupConfig::setThresholdValueAsPercentage(unsigned int percentageValue)
{
	unsigned int value = map(cz::clamp<unsigned int>(percentageValue, 0, 100), 0, 100, m_data.airValue, m_data.waterValue);
	if (value != m_data.thresholdValue)
	{
		m_isDirty = true;
		m_data.thresholdValue = value;
	}
}

unsigned int GroupConfig::getCurrentValue() const
{
	return m_currentValue;
}

unsigned int GroupConfig::getCurrentValueAsPercentage() const
{
	return map(m_currentValue, m_data.airValue, m_data.waterValue, 0, 100);
}

unsigned int GroupConfig::getAirValue() const
{
	return m_data.airValue;
}

unsigned int GroupConfig::getWaterValue() const
{
	return m_data.waterValue;
}

unsigned int GroupConfig::getSamplingInterval() const
{
	return m_data.samplingInterval;
}

unsigned int GroupConfig::getSamplingIntervalInMinutes() const
{
	return m_data.samplingInterval / 60;
}

void GroupConfig::setSamplingInterval(unsigned int value_)
{
	unsigned int value = cz::clamp<unsigned int>(value_, 1, MOISTURESENSOR_MAX_SAMPLINGINTERVAL);
	if (value != m_data.samplingInterval)
	{
		m_isDirty = true;
		m_data.samplingInterval = value;
	}
}

unsigned int GroupConfig::getShotDuration() const
{
	return m_data.shotDuration;
}

void GroupConfig::setShotDuration(unsigned int value_)
{
	unsigned int value = cz::clamp<unsigned int>(value_, 1, SHOT_MAX_DURATION);
	if (value != m_data.shotDuration)
	{
		m_isDirty = true;
		m_data.shotDuration = value;
	}
}

void GroupConfig::setSensorValue(unsigned int currentValue_, bool adjustRange)
{
	m_numReadings++;

	// If we are calibrating, we accept any value, and then adjust the air/water values accordingly
	if (adjustRange)
	{
		m_currentValue = currentValue_;
		if (m_currentValue > m_data.airValue)
		{
			m_isDirty = true;
			m_data.airValue = m_currentValue;
		}
		else if (m_currentValue < m_data.waterValue)
		{
			m_isDirty = true;
			m_data.waterValue = m_currentValue;
		}
	}
	else
	{
		m_currentValue = cz::clamp(currentValue_, m_data.waterValue, m_data.airValue);
	}
}

///////////////////////////////////////////////////////////////////////
// GroupData
///////////////////////////////////////////////////////////////////////

void GroupData::begin(uint8_t index)
{
	m_index = index;

// Fill the history with some values, for testing purposes
#if FASTER_ITERATION && 0
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
#else
	while(!m_history.isFull())
	{
		m_history.push({0, false});
	}
#endif
}

void GroupData::setMoistureSensorValues(const SensorReading& sample)
{
	if (m_inConfigMenu)
	{
		// If we are configuring this group, then we want to ignore the readings and just raise calibration events
		Component::raiseEvent(SoilMoistureSensorCalibrationReadingEvent(m_index, sample));
	}
	else if (m_cfg.isRunning())
	{
		m_cfg.setSensorValue(sample.meanValue, false);

		GraphPoint point = {0, 0, sample.status};
		point.val = map(m_cfg.getCurrentValue(), m_cfg.getAirValue(), m_cfg.getWaterValue(), 0, GRAPH_POINT_MAXVAL);

		// Since a motor can be turned on then off without a sensor reading in between, we use
		// m_pendingMotorPoint as a reminder there was a motor event, and so we'll draw that motor plot
		// on the next sensor reading
		point.motorOn = m_motorIsOn || m_pendingMotorPoint;
		m_pendingMotorPoint = false;
		if (m_history.isFull())
		{
			m_history.pop();
		}
		m_history.push(point);

		if (!sample.isValid())
		{
			m_sensorErrors++;
		}

		Component::raiseEvent(SoilMoistureSensorReadingEvent(m_index, sample));
	}
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
	if (m_cfg.isRunning() == state)
	{
		return;
	}

	// When we start or stop the group, we reset the error count
	// This allows the user to fix whatever is wrong and restart the group to get rid of the error
	m_sensorErrors = 0;

	m_cfg.setRunning(state);
	Component::raiseEvent(GroupOnOffEvent(m_index, state));
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
		m_cfg.save(dst);
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
		m_cfg.load(src);
	}

	if (loadHistory)
	{
		CZ_LOG(logDefault, Log, F("Loading group %d history from address %u"), m_index, src.getAddress());
		cz::load(src, m_history);
	}

	m_sensorErrors = 0;
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

void ProgramData::logConfig() const
{
	for(const GroupData& g : m_group)
	{
		g.logConfig();
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
	if (m_selectedGroup != -1 && getSelectedGroup()->isInConfigMenu())
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

GroupData& ProgramData::getGroupData(uint8_t index)
{
	CZ_ASSERT(index < MAX_NUM_PAIRS);
	return m_group[index];
}

void ProgramData::setTemperatureReading(float temperatureC)
{
	m_temperature = temperatureC;
	Component::raiseEvent(TemperatureSensorReadingEvent(m_temperature));
}

void ProgramData::setHumidityReading(float humidity)
{
	m_humidity = humidity;
	Component::raiseEvent(HumiditySensorReadingEvent(m_humidity));
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
	CZ_LOG(logDefault, Log, F("Saving full config to EEPROM, Took %u ms"), elapsedMs);
	Component::raiseEvent(ConfigSaveEvent());
}

void ProgramData::saveGroupConfig(uint8_t index)
{
	unsigned long startTime = micros();
	AT24C::Ptr ptr = m_outer.eeprom.at(0);

	for(uint8_t idx = 0; idx<index; ++idx)
	{
		ptr += m_group[idx].getConfigSaveSize();
	}

	uint16_t startAddress = ptr.getAddress();
	m_group[index].save(ptr, true, false);
	unsigned long elapsedMs = (micros() - startTime) / 1000;
	CZ_LOG(logDefault, Log, F("Saving group %u to EEPROM. Address %u, %u bytes. Took %u ms"),
		(unsigned int)index,
		startAddress,
		ptr.getAddress() - startAddress,
		elapsedMs);
	Component::raiseEvent(ConfigSaveEvent(index));
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
	CZ_LOG(logDefault, Log, F("Loading full config from EEPROM took %u ms"), elapsedMs);
	Component::raiseEvent(ConfigLoadEvent());
}

bool ProgramData::tryAcquireMuxMutex()
{
	if (m_muxMutex)
	{
		return false;
	}
	else
	{
		m_muxMutex = true;
		return true;
	}
}

void ProgramData::releaseMuxMutex()
{
	CZ_ASSERT(m_muxMutex==true);
	m_muxMutex = false;
}

} // namespace cz
