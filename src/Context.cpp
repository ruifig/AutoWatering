#include "Context.h"
#include "crazygaze/micromuc/Logging.h"
#include "crazygaze/micromuc/StringUtils.h"
#include "Component.h"
#include <Arduino.h>
#include <type_traits>

namespace cz
{

Context gCtx;

void updateEEPROM(ConfigStoragePtr& dst, const uint8_t* src, unsigned int size)
{
	while(size--)
	{
		dst.write(*src);
		++src;
	}
}

void readEEPROM(ConfigStoragePtr& src, uint8_t* dst, unsigned int size)
{
	while(size--)
	{
		*dst = src.read();
		++dst;
	}
}

void updateEEPROM(ConfigStoragePtr& dst, const char* src, unsigned int size)
{
	updateEEPROM(dst, reinterpret_cast<const uint8_t*>(src), size);
}

void readEEPROM(ConfigStoragePtr& src, char* dst, unsigned int size)
{
	readEEPROM(src, reinterpret_cast<uint8_t*>(dst), size);
}

template<typename T, typename = std::enable_if_t<
	std::is_arithmetic_v<T> ||
	std::is_same_v<T, GraphPoint>
	> >
void save(ConfigStoragePtr& dst, const T& v)
{
	updateEEPROM(dst, reinterpret_cast<const uint8_t*>(&v), sizeof(v));
}

template<typename T, typename = std::enable_if_t<
	std::is_arithmetic_v<T> ||
	std::is_same_v<T, GraphPoint>
	> >
void load(ConfigStoragePtr& src, T& v)
{
	readEEPROM(src, reinterpret_cast<uint8_t*>(&v), sizeof(v));
}

template<typename T>
void save(ConfigStoragePtr& src, const TFixedCapacityQueue<T>& v)
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
void load(ConfigStoragePtr& src, TFixedCapacityQueue<T>& v)
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

void GroupConfig::save(ConfigStoragePtr& dst) const
{
	if (m_isDirty)
	{
		m_isDirty = false;
		updateEEPROM(dst, reinterpret_cast<const uint8_t*>(&m_data), sizeof(m_data));
	}
	else
	{
		CZ_LOG(logDefault, Log, F("Group is not dirty. Nothing to save"));
		dst.inc(sizeof(m_data));
	}
}

void GroupConfig::load(ConfigStoragePtr& src)
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

float GroupConfig::getThresholdValueOnePercent() const
{
	return (m_data.airValue - m_data.waterValue) / 100.0f;
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
	unsigned int value = cz::clamp<unsigned int>(value_, 1, AW_MOISTURESENSOR_MAX_SAMPLINGINTERVAL);
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
	unsigned int value = cz::clamp<unsigned int>(value_, 1, AW_SHOT_MAX_DURATION);
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
	if (m_calibration.enabled)
	{
		if (currentValue_ < m_calibration.minValue)
		{
			m_calibration.minValue = currentValue_;
		}

		if (currentValue_ > m_calibration.maxValue)
		{
			m_calibration.maxValue = currentValue_;
		}

		if (m_data.airValue != m_calibration.maxValue)
		{
			m_isDirty = true;
			m_data.airValue = m_calibration.maxValue;
		}

		if (m_data.waterValue != m_calibration.minValue)
		{
			m_isDirty = true;
			m_data.waterValue = m_calibration.minValue;
		}
	}

	m_currentValue = cz::clamp(currentValue_, static_cast<unsigned int>(m_data.waterValue), static_cast<unsigned int>(m_data.airValue));
}

void GroupConfig::startCalibration()
{
	m_calibration.enabled = true;
	m_calibration.minValue = 1024;
	m_calibration.maxValue = 0;
	Component::raiseEvent(SoilMoistureSensorCalibrationEvent(getIndex(), true));
}

void GroupConfig::endCalibration()
{
	m_calibration.enabled = false;
	Component::raiseEvent(SoilMoistureSensorCalibrationEvent(getIndex(), false));
}

///////////////////////////////////////////////////////////////////////
// GroupData
///////////////////////////////////////////////////////////////////////

void GroupData::begin(uint8_t index)
{
	m_cfg.begin(index);


// Fill the history with some values, for testing purposes
#if AW_FASTER_ITERATION && 0
	for(int i=0; i<20; i++)
	{
		m_history.push({AW_TOUCHUI_GRAPH_POINT_MAXVAL, false});
	}

	for(int i=0; i<20; i++)
	{
		m_history.push({0, false});
	}

	for(int i=0; i<20; i++)
	{
		m_history.push({AW_TOUCHUI_GRAPH_POINT_MAXVAL/4, true});
	}

	for(int i=0; i<20; i++)
	{
		m_history.push({AW_TOUCHUI_GRAPH_POINT_MAXVAL/2 -1, false});
	}

	for(int i=0; i<20; i++)
	{
		m_history.push({AW_TOUCHUI_GRAPH_POINT_MAXVAL/2, true});
	}

	for(int i=0; i<20; i++)
	{
		m_history.push({(AW_TOUCHUI_GRAPH_POINT_MAXVAL*3)/4, false});
	}

	for(int i=0; i<20; i++)
	{
		m_history.push({AW_TOUCHUI_GRAPH_POINT_MAXVAL, true});
	}

	while(!m_history.isFull())
	{
		m_history.push({AW_TOUCHUI_GRAPH_POINT_MAXVAL/3, false});
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
		Component::raiseEvent(SoilMoistureSensorCalibrationReadingEvent(getIndex(), sample));
	}
	else if (m_cfg.isRunning())
	{
		m_cfg.setSensorValue(sample.meanValue, false);

		GraphPoint point = {0, 0, sample.status};
		point.val = map(m_cfg.getCurrentValue(), m_cfg.getAirValue(), m_cfg.getWaterValue(), 0, AW_TOUCHUI_GRAPH_POINT_MAXVAL);

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

		Component::raiseEvent(SoilMoistureSensorReadingEvent(getIndex(), sample));
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

	Component::raiseEvent(MotorEvent(getIndex(), m_motorIsOn));
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
	Component::raiseEvent(GroupOnOffEvent(getIndex(), state));
}

void GroupData::resetHistory()
{
	m_history.clear();
}

void GroupData::save(ConfigStoragePtr& dst, bool saveConfig, bool saveHistory) const
{
	if (saveConfig)
	{
		CZ_LOG(logDefault, Log, F("Saving group %d config at address %u"), getIndex(), dst.getAddress());
		m_cfg.save(dst);
	}

	if (saveHistory)
	{
		CZ_LOG(logDefault, Log, F("Saving group %d history at address %u"), getIndex(), dst.getAddress());
		cz::save(dst, m_history);
	}
}

void GroupData::load(ConfigStoragePtr& src, bool loadConfig, bool loadHistory)
{
	if (loadConfig)
	{
		CZ_LOG(logDefault, Log, F("Loading group %d config from address %u"), getIndex(), src.getAddress());
		m_cfg.load(src);
	}

	if (loadHistory)
	{
		CZ_LOG(logDefault, Log, F("Loading group %d history from address %u"), getIndex(), src.getAddress());
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
	CZ_LOG(logDefault, Log, F("DeviceName: %s"), m_devicename[0] ? m_devicename : formatString("NOT SET (using '%s')", gSetup->getDefaultName()));
	for(const GroupData& g : m_group)
	{
		g.logConfig();
	}
}

void ProgramData::setDeviceName(const char* name)
{
	// If no change, then nothing to do, so we don't spend time trying to save
	if (strcmp(m_devicename, name)==0)
	{
		return;
	}

	strncpy(m_devicename, name, AW_DEVICENAME_MAX_LEN);
	m_devicename[AW_DEVICENAME_MAX_LEN] = 0;
	constexpr int rebootDelay = 5000;
	save();
	CZ_LOG(logDefault, Log, "DeviceName set to '%s'. Rebooting in %d ms", m_devicename, rebootDelay);
	delay(rebootDelay);
	rp2040.reboot();
}

const char* ProgramData::getDeviceName()
{
	return m_devicename[0] ? m_devicename : gSetup->getDefaultName();
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
	CZ_ASSERT(index < AW_MAX_NUM_PAIRS);
	return m_group[index];
}

void ProgramData::setTemperatureReading(float temperatureC)
{
	// Temperature is rounded to XXX.X , so we can ignore repeated readings with the same value and thus
	// not raise events when nothing changed
	temperatureC = static_cast<int>(temperatureC * 10) / 10.0f;

	if (!cz::isNearlyEqual(m_temperature, temperatureC))
	{
		m_temperature = temperatureC;
		Component::raiseEvent(TemperatureSensorReadingEvent(m_temperature));
	}
}

void ProgramData::setHumidityReading(float humidity)
{
	// Humidity is rounded to XXX.X , so we can ignore repeated readings with the same value and thus
	// not raise events when nothing changed
	humidity = static_cast<int>(humidity*10) / 10.0f;

	if (!cz::isNearlyEqual(m_humidity, humidity))
	{
		m_humidity = humidity;
		Component::raiseEvent(HumiditySensorReadingEvent(m_humidity));
	}
}

void ProgramData::save() const
{
	m_outer.configStorage.start();

	unsigned long startTime = micros();
	ConfigStoragePtr ptr = m_outer.configStorage.ptrAt(0);

	CZ_LOG(logDefault, Log, F("Saving full config. DeviceName: %s"), m_devicename);
	updateEEPROM(ptr, m_devicename, sizeof(m_devicename));

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
	
	m_outer.configStorage.end();

	unsigned long elapsedMs = (micros() - startTime) / 1000;
	CZ_LOG(logDefault, Log, F("Saving full config to EEPROM, Took %u ms"), elapsedMs);
	bool wasReady = m_isReady;
	m_isReady = true;
	if (!wasReady)
	{
		Component::raiseEvent(ConfigReadyEvent());
	}

	Component::raiseEvent(ConfigSaveEvent());
}

void ProgramData::saveGroupConfig(uint8_t index)
{
	m_outer.configStorage.start();

	unsigned long startTime = micros();
	ConfigStoragePtr ptr = m_outer.configStorage.ptrAt(0);

	ptr.inc(sizeof(m_devicename));

	for(uint8_t idx = 0; idx<index; ++idx)
	{
		ptr.inc(m_group[idx].getConfigSaveSize());
	}

	uint16_t startAddress = ptr.getAddress();
	m_group[index].save(ptr, true, false);

	m_outer.configStorage.end();

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

	m_outer.configStorage.start();
	ConfigStoragePtr ptr = m_outer.configStorage.ptrAt(0);

	readEEPROM(ptr, m_devicename, sizeof(m_devicename));
	CZ_LOG(logDefault, Log, F("Loading config. DeviceName: %s"), m_devicename);

	for(GroupData& g : m_group)
	{
		g.load(ptr, true, false);
	}

	for(GroupData& g : m_group)
	{
		g.load(ptr, false, true);
	}

	m_outer.configStorage.end();
	
	unsigned long elapsedMs = (micros() - startTime) / 1000;
	CZ_LOG(logDefault, Log, F("Loading full config from EEPROM took %u ms"), elapsedMs);

	logConfig();	

	bool wasReady = m_isReady;
	m_isReady = true;
	if (!wasReady)
	{
		Component::raiseEvent(ConfigReadyEvent());
	}

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
