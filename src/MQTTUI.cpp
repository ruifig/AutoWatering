#include "MQTTUI.h"
#include "SoilMoistureSensor.h"

CZ_DEFINE_LOG_CATEGORY(logMQTTUI);

namespace cz
{

#if AW_MQTTUI_ENABLED
	MQTTUI gMQTTUI;
#endif

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

void MQTTUI::onEvent(const Event& evt)
{
	switch(evt.type)
	{
		case Event::Type::ConfigReady:
		{
			if (!m_subscribed)
			{
				m_subscribed = true;
				MQTTCache* mqtt = MQTTCache::getInstance();
				mqtt->subscribe(formatString("%s/groups/%s", ADAFRUIT_IO_USERNAME, gCtx.data.getDeviceName()));
				mqtt->setListener(*this);

				m_deviceNameValue = mqtt->set(buildFeedName("devicename"), gCtx.data.getDeviceName(), 2, false);

				for(int i=0; i<AW_MAX_NUM_PAIRS; i++)
				{
					SoilMoistureSensor* sensor = gSetup->getSoilMoistureSensor(i);
					GroupData& groupData = gCtx.data.getGroupData(i);
					#if 0
					mqtt->set(buildFeedName("group", i, "running"), groupData.isRunning() ? 1 : 0, 2, false);
					mqtt->set(buildFeedName("group", i, "samplinginterval"), groupData.getSamplingInterval(), 2, false);
					mqtt->set(buildFeedName("group", i, "shotduration"), groupData.getShotDuration(), 2, false);
					mqtt->set(buildFeedName("group", i, "airvalue"), groupData.getAirValue(), 2, false);
					mqtt->set(buildFeedName("group", i, "watervalue"), groupData.getWaterValue(), 2, false);
					mqtt->set(buildFeedName("group", i, "threshold"), groupData.getThresholdValueAsPercentage(), 2, false);
					#endif
				}
			}
		}
		break;

		case Event::TemperatureSensorReading:
		{
			auto&& e = static_cast<const TemperatureSensorReadingEvent&>(evt);
			MQTTCache::getInstance()->set<1>(buildFeedName("temperature"), e.temperatureC, 2, false);
		}
		break;

		case Event::HumiditySensorReading:
		{
			auto&& e = static_cast<const HumiditySensorReadingEvent&>(evt);
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
			MQTTCache::getInstance()->set(
				buildFeedName("battery", "perc"),
				e.percentage, 2, false);
			MQTTCache::getInstance()->set<2>(
				buildFeedName("battery", "voltage"),
				e.voltage, 2, false);
		}
		break;
	}
} 

void MQTTUI::onMqttValueReceived(const MQTTCache::Entry* entry)
{
	if (entry == m_deviceNameValue)
	{
		gCtx.data.setDeviceName(entry->value.c_str());
	}
}

bool MQTTUI::processCommand(const Command& cmd)
{
	return false;
}

} // namespace cz
