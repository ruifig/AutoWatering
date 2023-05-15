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
}

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
	else
	{
		return true;
	}
}

float MQTTUI::tick(float deltaSeconds)
{
	if (!m_configSent)
	{
		// We want to discard the first tick because deltaSeconds will be huge after the Wifi connected event
		if (m_firstTick)
		{
			m_firstTick = false;
		}
		else
		{
			m_waitingForConfigTimeout -= deltaSeconds;
			if (m_waitingForConfigTimeout <= 0)
			{
				publishConfig();
			}
		}
	}

	return 0.1f;
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

void MQTTUI::publishGroupConfig(int index)
{
	MQTTCache* mqtt = MQTTCache::getInstance();
	GroupData& groupData = gCtx.data.getGroupData(index);
	mqtt->set(buildFeedName("group", index, "running"), groupData.isRunning() ? 1 : 0, 2, false);
	mqtt->set(buildFeedName("group", index, "samplinginterval"), groupData.getSamplingIntervalInMinutes(), 2, false);
	mqtt->set(buildFeedName("group", index, "shotduration"), groupData.getShotDuration(), 2, false);
	mqtt->set(buildFeedName("group", index, "airvalue"), groupData.getAirValue(), 2, false);
	mqtt->set(buildFeedName("group", index, "watervalue"), groupData.getWaterValue(), 2, false);
	mqtt->set(buildFeedName("group", index, "threshold"), groupData.getThresholdValueAsPercentage(), 2, false);
}

void MQTTUI::publishConfig()
{
	if (m_configSent)
	{
		return;
	}

	MQTTCache* mqtt = MQTTCache::getInstance();

	m_deviceNameValue = mqtt->set(buildFeedName("devicename"), gCtx.data.getDeviceName(), 2, false);

	for (int i = 0; i < AW_MAX_NUM_PAIRS; i++)
	{
		publishGroupConfig(i);
		mqtt->set(buildFeedName("group", i, "motoron"), 0, 2, false);
	}

	m_configSent = true;
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

				if (!m_subscribed)
				{
					m_subscribed = true;
					MQTTCache* mqtt = MQTTCache::getInstance();
					mqtt->subscribe(formatString("%s/groups/%s", ADAFRUIT_IO_USERNAME, gCtx.data.getDeviceName()));
					mqtt->setListener(*this);

					// We'll wait a bit to see if we receive the entire feed group
					if (!m_configSent)
					{
						m_firstTick = true;
						m_waitingForConfigTimeout = 5.0f;
					}
				}
			}
			else
			{
				m_subscribed = false;
				m_configSent = false;
				m_waitingForConfigTimeout = 0.0f;
				m_firstTick = true;
			}
		}
		break;

		case Event::TemperatureSensorReading:
		{
			auto&& e = static_cast<const TemperatureSensorReadingEvent&>(evt);
			// We don't need to wait for Wifi to connect to set this
			MQTTCache::getInstance()->set<1>(buildFeedName("temperature"), e.temperatureC, 2, AW_MQTT_SENSOR_FORCESYNC ? true : false);
		}
		break;

		case Event::HumiditySensorReading:
		{
			auto&& e = static_cast<const HumiditySensorReadingEvent&>(evt);
			// We don't need to wait for Wifi to connect to set this
			MQTTCache::getInstance()->set<1>(buildFeedName("humidity"), e.humidity, 2, AW_MQTT_SENSOR_FORCESYNC ? true : false);
		}
		break;

		case Event::SoilMoistureSensorReading:
		{
			auto&& e = static_cast<const SoilMoistureSensorReadingEvent&>(evt);
			const GroupData& data = gCtx.data.getGroupData(e.index);
			SoilMoistureSensor* sensor = gSetup->getSoilMoistureSensor(e.index);
			if (e.reading.isValid())
			{
				// We don't need to wait for Wifi to connect to set this
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
			auto&& e = static_cast<const SoilMoistureSensorCalibrationEvent&>(evt);
			if (!e.started) // check if we are starting or finishing the calibration
			{
				publishGroupConfig(e.index);
			}
		}
		break;

		case Event::GroupOnOff:
		{
			auto&& e = static_cast<const GroupOnOffEvent&>(evt);
			publishGroupConfig(e.index);
		}
		break;

		case Event::BatteryLifeReading:
		{
			auto&& e = static_cast<const BatteryLifeReadingEvent&>(evt);
			// We don't need to wait for Wifi to connect to set these
			MQTTCache::getInstance()->set(
				buildFeedName("battery", "perc"),
				e.percentage, 2, false);
			MQTTCache::getInstance()->set<2>(
				buildFeedName("battery", "voltage"),
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

	if (entry == m_deviceNameValue)
	{
		gCtx.data.setDeviceName(entry->value.c_str());

		// If we are receiving the devicename for the first time, then it's very likely part of the initial message with the entire feed group.
		// In that case, we reset the timer, so the next tick we publish the full config. That needs to be done from the tick and NOT from here,
		// because at this point MQTTCache is still processing the received feed group message.
		// By waiting until the tick, it means MQTTCache will have created all the entries from the feed group message.
		if (!m_configSent)
		{
			m_waitingForConfigTimeout = 0.0f;
		}
	}
	else if (parseTopic(entry->topic.c_str(), parsed))
	{
		if (parsed.index==-1) // -1 means it's not part of a sensor/motor group
		{
			//
		}
		else
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
				// NOTE: Both the touch UI and MQTT show sampling intervals in minutes, but internall it uses seconds
				data.setSamplingInterval(atoi(entry->value.c_str())*60);
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


			if (data.isDirty())
			{
				gCtx.data.saveGroupConfig(data.getIndex());
			}
		}
	}
}

bool MQTTUI::processCommand(const Command& cmd)
{
	return false;
}

} // namespace cz
