#include "MQTTUI.h"
#include "SoilMoistureSensor.h"
#include "Timer.h"
#include "PumpMonitor.h"

CZ_DEFINE_LOG_CATEGORY(logMQTTUI);

namespace cz
{

extern Timer gTimer;

#if AW_MQTTUI_ENABLED
	MQTTUI gMQTTUI;
#endif

namespace
{

	struct ParsedTopic
	{
		// Full topic name
		const char* fullTopicName = nullptr;
		// If -1, then it's part of a config group
		int index = -1;
		bool thisDevice = false;
		// Name of the actual entry (removing the devicename and group if in a group)
		const char* name = nullptr; 
	};

	//
	bool parseTopic(const char* fullTopicName, ParsedTopic& dst)
	{
		ParsedTopic parsed;
		const char* dotPos = strrchr(fullTopicName, '.');
		const char* deviceNamePos = strrchr(fullTopicName, '/');
		if (!dotPos || !deviceNamePos)
		{
			return false;
		}
		deviceNamePos++;

		parsed.fullTopicName = fullTopicName;
		parsed.name = dotPos + 1;

		if (strncmp(deviceNamePos, gCtx.data.getDeviceName(), dotPos - deviceNamePos)==0)
		{
			parsed.thisDevice = true;
			if (strstr(parsed.name, "group") == parsed.name)
			{
				parsed.name += strlen("group");
				parsed.index = atoi(parsed.name);
				if (const char* dashPos = strchr(parsed.name, '-'))
				{
					parsed.name = dashPos + 1;
					dst = parsed;
					return true;
				}
				else
				{
					return false;
				}
			}
			else
			{
				dst = parsed;
				return true;
			}
		}
		else
		{
			return false;
		}
	}

	const char* buildFeedName(const char* part1, int index, const char* part2)
	{
		return formatString(ADAFRUIT_IO_USERNAME"/feeds/%s.%s%d-%s", gCtx.data.getDeviceName(), part1, index, part2);
	}

	const char* buildFeedName(const char* part1, const char* part2)
	{
		return formatString(ADAFRUIT_IO_USERNAME"/feeds/%s.%s-%s", gCtx.data.getDeviceName(), part1, part2);
	}

	const char* buildFeedName(const char* part1)
	{
		return formatString(ADAFRUIT_IO_USERNAME"/feeds/%s.%s", gCtx.data.getDeviceName(), part1);
	}

}

const char* const MQTTUI::ms_stateNames[4] =
{
	"WaitingForConnection",
	"WaitingForConfig",
	"Idle",
	"CalibratingSensor"
};

MQTTUI::MQTTUI()
{
	// We only start ticking when we ready a ConfigReady event
	stopTicking();
}

bool MQTTUI::initImpl()
{
	MQTTCache* mqttCache = MQTTCache::getInstance();
	// If we are using MQTTUI, then by design there must exist an MQTTCache instance.
	CZ_ASSERT(mqttCache);

	if (!mqttCache->isInitialized())
	{
		return false;
	}

	onEnterState();
	return true;
}

float MQTTUI::tick(float deltaSeconds)
{
	constexpr float retVal = 0.1f;

	if (m_ignoreNextTick)
	{
		m_ignoreNextTick = false;
		return retVal;
	}

	m_timeInState += deltaSeconds;

	if (m_saveDelay > 0.0f)
	{
		m_saveDelay -= deltaSeconds;
		if (m_saveDelay <= 0.0f)
		{
			CZ_LOG(logMQTTUI, Log, "Save delay timer expired. Performing save");
			gCtx.data.save();
		}
	}

	switch(m_state)
	{
		case WaitingForConnection:
		break;

		case WaitingForConfig:
		{
			if (m_timeInState >= AW_MQTTUI_WAITFORCONFIG_TIMEOUT)
			{
				CZ_LOG(logMQTTUI, Log, "Config not received.")
				publishConfig();
				changeToState(State::Idle);
			}
		}
		break;

		case Idle:
		break;

		case CalibratingSensor:
		break;
	};

	return retVal;
}

void MQTTUI::publishGroupData(int index)
{
	MQTTCache* mqtt = MQTTCache::getInstance();
	GroupData& groupData = gCtx.data.getGroupData(index);
	mqtt->set(buildFeedName("group", index, "running"), groupData.isRunning() ? 1 : 0, 2, false);
	mqtt->set(buildFeedName("group", index, "motoron"), 0, 2, false);
	mqtt->set(buildFeedName("group", index, "samplinginterval"), groupData.getSamplingIntervalInMinutes(), 2, false);
	mqtt->set(buildFeedName("group", index, "shotduration"), groupData.getShotDuration(), 2, false);
	mqtt->set(buildFeedName("group", index, "threshold"), groupData.getThresholdValueAsPercentage(), 2, false);
	mqtt->set(buildFeedName("group", index, "value"), groupData.getCurrentValueAsPercentage(), 2, false);
}

String MQTTUI::createConfigJson()
{
	DynamicJsonDocument& doc = *MQTTCache::getInstance()->getScratchJsonDocument();
	doc.clear();

	doc["devicename"] = gCtx.data.getDeviceName();

	JsonArray groups = doc.createNestedArray("groups");
	for(int i=0; i< AW_MAX_NUM_PAIRS; i++)
	{
		GroupData& groupData = gCtx.data.getGroupData(i);
		JsonObject obj = groups.createNestedObject();
		obj["running"] = groupData.isRunning() ? 1 : 0;
		obj["samplingInterval"] = groupData.getSamplingInterval();
		obj["shotDuration"] = groupData.getShotDuration();
		obj["waterValue"] = groupData.getWaterValue();
		obj["airValue"] = groupData.getAirValue();
		obj["thresholdValue"] = groupData.getThresholdValue();
	}

	CZ_LOG(logMQTTUI, Verbose, "DynamicJsonDocument memory usage: %u bytes out of %u", doc.memoryUsage(), doc.capacity());
	String res;
#if CZ_LOG_ENABLED
	serializeJsonPretty(doc, res);
	cz::LogOutput::logToAllSimple(res.c_str());
	res = "";
#endif

	serializeJson(doc, res);
	doc.clear();
	return res;
}

void MQTTUI::publishConfig()
{
	CZ_LOG(logMQTTUI, Log, "Sending local config");

	auto createIfNotExists = [](const char* topic, auto value)
	{
		MQTTCache* mqtt = MQTTCache::getInstance();
		if (mqtt->get(topic) == nullptr)
		{
			mqtt->set(topic, value, 2, false);
		}
	};

	MQTTCache* mqtt = MQTTCache::getInstance();
	mqtt->set(buildFeedName("fullconfig"), createConfigJson().c_str(), 2, false);

	// Create feeds that don't exist
	createIfNotExists(buildFeedName("devicename"), gCtx.data.getDeviceName());
	if constexpr(AW_BATTERYLIFE_ENABLED)
	{
		createIfNotExists(buildFeedName("battery-perc"), 0);
		createIfNotExists(buildFeedName("battery-voltage"), 0);
	}

	if constexpr(AW_THSENSOR_ENABLED)
	{
		createIfNotExists(buildFeedName("humidity"), 0);
		createIfNotExists(buildFeedName("temperature"), 0);
	}

	createIfNotExists(buildFeedName("calibration", "cancel"), 0);
	createIfNotExists(buildFeedName("calibration", "index"), -1);
	createIfNotExists(buildFeedName("calibration", "info"), "");
	createIfNotExists(buildFeedName("calibration", "reset"), 0);
	createIfNotExists(buildFeedName("calibration", "save"), 0);
	createIfNotExists(buildFeedName("calibration", "threshold"), 0);

	for (int i = 0; i < AW_MAX_NUM_PAIRS; i++)
	{
		publishGroupData(i);
	}

	publishCalibrationInfo();
}

void MQTTUI::onEvent(const Event& evt)
{
	switch(evt.type)
	{
		case Event::Type::ConfigReady:
		{
		}
		break;

		case Event::Type::WifiStatus:
		{
			auto&& e = static_cast<const WifiStatusEvent&>(evt);
			if (e.connected)
			{
				startTicking();
				changeToState(State::WaitingForConfig);
			}
			else
			{
				changeToState(State::WaitingForConnection);
			}
		}
		break;

		case Event::TemperatureSensorReading:
		{
			// NOTE: We can set the cache entry for this regardless of the state
			auto&& e = static_cast<const TemperatureSensorReadingEvent&>(evt);
			// We don't need to wait for Wifi to connect to set this
			MQTTCache::getInstance()->set<1>(buildFeedName("temperature"), e.temperatureC, 2, AW_MQTT_SENSOR_FORCESYNC ? true : false);
		}
		break;

		case Event::HumiditySensorReading:
		{
			// NOTE: We can set the cache entry for this regardless of the state
			auto&& e = static_cast<const HumiditySensorReadingEvent&>(evt);
			// We don't need to wait for Wifi to connect to set this
			MQTTCache::getInstance()->set<1>(buildFeedName("humidity"), e.humidity, 2, AW_MQTT_SENSOR_FORCESYNC ? true : false);
		}
		break;

		case Event::SoilMoistureSensorReading:
		{
			// NOTE: We can set the cache entry for this regardless of the state. E.g: Doesn't matter if Wifi is not connected yet.

			auto&& e = static_cast<const SoilMoistureSensorReadingEvent&>(evt);
			const GroupData& data = gCtx.data.getGroupData(e.index);
			SoilMoistureSensor* sensor = gSetup->getSoilMoistureSensor(e.index);
			if (e.reading.isValid())
			{
				MQTTCache* mqtt = MQTTCache::getInstance();
				const char* topicName = buildFeedName("group", e.index, "value");
				const MQTTCache::Entry* entry = mqtt->get(topicName);
				bool forceSync = AW_MQTT_SENSOR_FORCESYNC ? true : false;
				// If the sensor is setup to do very fast readings, then we don't want to send all of them if the value didn't change. We send every X seconds
				if (entry)
				{
					// Only publish if we enough time passed since the last publish
					if ((gTimer.getTotalSeconds() - entry->lastSyncTime) >= AW_MQTT_MOISTURESENSOR_MININTERVAL)
					{
						mqtt->set<0>(entry, data.getCurrentValueAsPercentage(), 2, forceSync);
					}
				}
				else
				{
					mqtt->set<0>(topicName, data.getCurrentValueAsPercentage(), 2, forceSync);
				}
			}
			else
			{
				// #TODO : We should log problems to some feed
				CZ_LOG(logMQTTUI, Warning, "Soil moisture reading for pair %d is invalid (%s)", static_cast<int>(e.index), e.reading.getStatusText());
			}
		}
		break;

		case Event::SoilMoistureSensorCalibration:
		{
			if (m_state == State::CalibratingSensor)
			{
				auto&& e = static_cast<const SoilMoistureSensorCalibrationEvent&>(evt);
				if (e.started)
				{
					// Nothing to do
				}
				else
				{
					// If finished the calibration, then publish the new group data
					publishGroupData(e.index);
				}
			}
		}
		break;

		case Event::SoilMoistureSensorCalibrationReading:
		{
			if (m_state == State::CalibratingSensor)
			{
				auto& e = static_cast<const SoilMoistureSensorCalibrationReadingEvent&>(evt);
				if (e.index==m_calibratingIndex)
				{
					if (e.reading.isValid())
					{
						m_dummyCfg.setSensorValue(e.reading.meanValue, true);
						publishCalibrationInfo();
					}
				}
			}
		}
		break;

		case Event::GroupOnOff:
		{
			auto&& e = static_cast<const GroupOnOffEvent&>(evt);
			publishGroupData(e.index);
		}
		break;

		case Event::BatteryLifeReading:
		{
			auto&& e = static_cast<const BatteryLifeReadingEvent&>(evt);
			// We don't need to wait for Wifi to connect to set these
			MQTTCache::getInstance()->set(
				buildFeedName("battery-perc"),
				e.percentage, 2, false);
			MQTTCache::getInstance()->set<2>(
				buildFeedName("battery-voltage"),
				e.voltage, 2, false);
		}
		break;

		case Event::Motor:
		{
			auto&& e = static_cast<const MotorEvent&>(evt);
			// NOTE: Instead of 0-1, we use 0-100.
			// This is so that the user can setup the MQTT dashboard to show the moisture sensor value (0-100) and the motor on/off in the same chart.
			// To make it look nicer, if using AdafruitIO, the user should enable the Line Chart's "Stepped Line".
			MQTTCache::getInstance()->set(buildFeedName("group", e.index, "motoron"), e.started ? 100 : 0, 2, true);
		}
		break;
	}
} 

void MQTTUI::onMqttValueReceived(const MQTTCache::Entry* entry)
{
	ParsedTopic parsed;

	if (!parseTopic(entry->topic.c_str(), parsed))
	{
		return;
	}

	// Regardless of the state, if we receive a "devicename" change, then we update the local devicename, which will cause a reboot
	if (strcmp(parsed.name, "devicename") == 0)
	{
		gCtx.data.setDeviceName(entry->value.c_str());
	}

	if (m_state == State::WaitingForConfig)
	{
		if (strcmp(parsed.name, "fullconfig") == 0)
		{
			CZ_LOG(logDefault, Log, "**** TODO **** process received config")
			changeToState(State::Idle);
			// TODO : Start the save delay timer, or save immediately
		}
	}
	else if (m_state == State::Idle)
	{
		if (parsed.index == -1)
		{
			if (strcmp(parsed.name, "calibration-index") == 0)
			{
				int value = atoi(entry->value.c_str());

				// 0..N : Group to start calibrating
				// -1 : Button released
				if (value != -1)
				{
					startCalibration(value);
					changeToState(State::CalibratingSensor);
				}
			}
		}
		else  // Check if this feed is part of a group
		{
			GroupData& data = gCtx.data.getGroupData(parsed.index);
			if (strcmp(parsed.name, "running") == 0)
			{
				data.setRunning(atoi(entry->value.c_str()) == 0 ? false : true);
			}
			else if (strcmp(parsed.name, "threshold") == 0)
			{
				data.setThresholdValueAsPercentage(atoi(entry->value.c_str()));
			}
			else if (strcmp(parsed.name, "samplinginterval") == 0)
			{
				// NOTE: Both the touch UI and MQTT show sampling intervals in minutes, but internall it uses
				// seconds
				data.setSamplingInterval(atoi(entry->value.c_str()) * 60);
			}
			else if (strcmp(parsed.name, "shotduration") == 0)
			{
				data.setShotDuration(atoi(entry->value.c_str()));
			}
			else if (strcmp(parsed.name, "motoron") == 0)
			{
				if (atoi(entry->value.c_str()) != 0)
				{
					gSetup->getPumpMonitor(parsed.index)->doShot();
				}
			}
			else if (strcmp(parsed.name, "calibrate") == 0)
			{
				if (atoi(entry->value.c_str()) != 0)
				{
					startCalibration(parsed.index);
					changeToState(State::CalibratingSensor);
				}
			}

			if (data.isDirty())
			{
				if (m_saveDelay <= 0.0f)
				{
					CZ_LOG(logMQTTUI, Log, "Starting save delay timer (%0.2f)", (float)AW_MQTTUI_SAVEDELAY);
				}

				m_saveDelay = AW_MQTTUI_SAVEDELAY;
			}
		}
	}
	else if (m_state == State::CalibratingSensor)
	{
		if (parsed.index == -1)  // -1 means it's not part of a sensor/motor group
		{
			if (strcmp(parsed.name, "calibration-reset") == 0)
			{
				resetCalibration();
			}
			else if (strcmp(parsed.name, "calibration-cancel") == 0)
			{
				cancelCalibration();
				changeToState(State::Idle);
			}
			else if (strcmp(parsed.name, "calibration-save") == 0)
			{
				saveCalibration();
				changeToState(State::Idle);
			}
			else if (strcmp(parsed.name, "calibration-threshold") == 0)
			{
				m_dummyCfg.setThresholdValueAsPercentage(atoi(entry->value.c_str()));
			}
		}
	}

}

void MQTTUI::publishCalibrationInfo()
{
	MQTTCache* mqtt = MQTTCache::getInstance();
	if (m_calibratingIndex == -1)
	{
		mqtt->set(buildFeedName("calibration", "info"), formatString("NO GROUP SELECTED"), 1, true);
		mqtt->set(buildFeedName("calibration", "threshold"), 0, 1, false);
	}
	else
	{
		mqtt->set(
			buildFeedName("calibration", "info"),
			formatString("Group %d: Air(%d)  Reading(%d%%) Water(%d)", m_calibratingIndex, m_dummyCfg.getAirValue(), m_dummyCfg.getCurrentValueAsPercentage(), m_dummyCfg.getWaterValue()),
			1, true);
		mqtt->set(buildFeedName("calibration", "threshold"), m_dummyCfg.getThresholdValueAsPercentage(), 1, false);
		// To make sure we don't leave a group marked as "calibrating", otherwise when we boot and receive a '1' for a group,
		// it will go into calibrating mode and thus that group won't be monitoring.
		mqtt->set(buildFeedName("group", m_calibratingIndex, "calibrate"), 0, 2, false);
	}
}

void MQTTUI::cancelCalibration()
{
	if (m_calibratingIndex == -1)
		return;

	GroupData& data = gCtx.data.getGroupData(m_calibratingIndex);
	data.setInConfigMenu(false);
	m_calibratingIndex = -1;
	publishCalibrationInfo();
}

void MQTTUI::startCalibration(int index)
{
	cancelCalibration();

	m_calibratingIndex = index;
	GroupData& data = gCtx.data.getGroupData(index);
	m_dummyCfg = data.copyConfig();
	data.setInConfigMenu(true);

	publishCalibrationInfo();
}

void MQTTUI::resetCalibration()
{
	if (m_calibratingIndex == -1)
		return;

	m_dummyCfg.startCalibration();
	publishCalibrationInfo();
}

void MQTTUI::saveCalibration()
{
	if (m_calibratingIndex == -1)
		return;

	GroupData& data = gCtx.data.getGroupData(m_calibratingIndex);
	data.setConfig(m_dummyCfg);
	m_dummyCfg.endCalibration();

	cancelCalibration();
}

bool MQTTUI::processCommand(const Command& cmd)
{
	return false;
}

void MQTTUI::changeToState(State newState)
{
	CZ_LOG(logMQTTUI, Log, F("Changing state: %ssec %s->%s")
		, *FloatToString(m_timeInState)
		, ms_stateNames[(int)m_state]
		, ms_stateNames[(int)newState]);

	onLeaveState();
	m_state = newState;
	m_timeInState = 0.0f;
	onEnterState();
}

void MQTTUI::setSubscriptions(bool config, bool group)
{
	auto process = [](bool subscribe, const char* topic)
	{
		if (subscribe)
		{
			MQTTCache::getInstance()->subscribe(topic);
		}
		else
		{
			MQTTCache::getInstance()->unsubscribe(topic);
		}
	};

	process(config, formatString("%s/feeds/%s.fullconfig", ADAFRUIT_IO_USERNAME, gCtx.data.getDeviceName()));
	process(group, formatString("%s/groups/%s/json", ADAFRUIT_IO_USERNAME, gCtx.data.getDeviceName()));

}

void MQTTUI::onLeaveState()
{
	switch(m_state)
	{
		case WaitingForConnection:
		break;

		case WaitingForConfig:
		{
			setSubscriptions(false, false);
		}
		break;

		case Idle:
		break;

		case CalibratingSensor:
		break;
	}
}

void MQTTUI::onEnterState()
{
	switch(m_state)
	{
		case WaitingForConnection:
			setSubscriptions(false, false);
		break;

		case WaitingForConfig:
		{
			// We enter this state as the result of a wifi connect, which takes a very long time and thus the next tick will have a huge delta.
			// Therefore we ignore the next tick
			m_ignoreNextTick = true;
			MQTTCache::getInstance()->setListener(*this);
			setSubscriptions(true, false);
		}
		break;

		case Idle:
		{
			setSubscriptions(false, true);
		}
		break;

		case CalibratingSensor:
		break;
	}
}

} // namespace cz
