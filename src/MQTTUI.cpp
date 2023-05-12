#include "MQTTUI.h"
#include "SoilMoistureSensor.h"

CZ_DEFINE_LOG_CATEGORY(logMQTTUI);

namespace cz
{

#if AW_MQTTUI_ENABLED
	MQTTUI gMQTTUI;
#endif

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
		SoilMoistureSensor* sensor = gSetup->getSoilMoistureSensor(i);
		GroupData& groupData = gCtx.data.getGroupData(i);
#if 1
		mqtt->set(buildFeedName("group", i, "running"), groupData.isRunning() ? 1 : 0, 2, false);
		mqtt->set(buildFeedName("group", i, "samplinginterval"), groupData.getSamplingInterval(), 2, false);
		mqtt->set(buildFeedName("group", i, "shotduration"), groupData.getShotDuration(), 2, false);
		mqtt->set(buildFeedName("group", i, "airvalue"), groupData.getAirValue(), 2, false);
		mqtt->set(buildFeedName("group", i, "watervalue"), groupData.getWaterValue(), 2, false);
		mqtt->set(buildFeedName("group", i, "threshold"), groupData.getThresholdValueAsPercentage(), 2, false);
		mqtt->set(buildFeedName("group", i, "motoron"), 0, 2, false);
#endif
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
			MQTTCache::getInstance()->set<1>(buildFeedName("temperature"), e.temperatureC, 2, false);
		}
		break;

		case Event::HumiditySensorReading:
		{
			auto&& e = static_cast<const HumiditySensorReadingEvent&>(evt);
			// We don't need to wait for Wifi to connect to set this
			MQTTCache::getInstance()->set<1>(buildFeedName("humidity"), e.humidity, 2, false);
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
				MQTTCache::getInstance()->set<0>(
					buildFeedName("group", e.index, "value"),
					data.getCurrentValueAsPercentage(), 2, false);
			}
			else
			{
				// #TODO : We should log problems to some feed
				CZ_LOG(logMQTTUI, Warning, "Soil moisture reading for pair %d is invalid (%s)", static_cast<int>(e.index), e.reading.getStatusText());
			}
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
			MQTTCache::getInstance()->set(buildFeedName("group", e.index, "motoron"), e.started ? 1 : 0, 2, true);
		}
		break;
	}
} 

void MQTTUI::onMqttValueReceived(const MQTTCache::Entry* entry)
{
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
}

bool MQTTUI::processCommand(const Command& cmd)
{
	return false;
}

} // namespace cz
