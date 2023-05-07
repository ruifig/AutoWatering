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

void MQTTUI::onEvent(const Event& evt)
{
	switch(evt.type)
	{
		case Event::Type::ConfigLoad:
		{
			if (!m_subscribed)
			{
				m_subscribed = true;
				MQTTCache::getInstance()->subscribe(formatString("%s/groups/%s", ADAFRUIT_IO_USERNAME, gCtx.data.getDeviceName()));
				MQTTCache::getInstance()->setListener(*this);
			}
		}
		break;

		case Event::TemperatureSensorReading:
		{
			auto&& e = static_cast<const TemperatureSensorReadingEvent&>(evt);
			MQTTCache::getInstance()->set<1>(formatString(ADAFRUIT_IO_USERNAME"/feeds/%s.temperature", gCtx.data.getDeviceName()), e.temperatureC, 2, false);
		}
		break;

		case Event::HumiditySensorReading:
		{
			auto&& e = static_cast<const HumiditySensorReadingEvent&>(evt);
			MQTTCache::getInstance()->set<1>(formatString(ADAFRUIT_IO_USERNAME"/feeds/%s.humidity", gCtx.data.getDeviceName()), e.humidity, 2, false);
		}
		break;

		case Event::SoilMoistureSensorReading:
		{
			auto&& e = static_cast<const SoilMoistureSensorReadingEvent&>(evt);
			const GroupData& data = gCtx.data.getGroupData(e.index);
			SoilMoistureSensor* sensor = gSetup->getSoilMoistureSensor(e.index);
			if (e.reading.isValid())
			{
				MQTTCache::getInstance()->set<1>(
					formatString(ADAFRUIT_IO_USERNAME"/feeds/%s.%s-value", gCtx.data.getDeviceName(), sensor->getName()),
					data.getCurrentValueAsPercentage(), 2, false);

				MQTTCache::getInstance()->set<1>(
					formatString(ADAFRUIT_IO_USERNAME"/feeds/%s.%s-threshold", gCtx.data.getDeviceName(), sensor->getName()),
					data.getThresholdValueAsPercentage(), 2, false);
			}
			else
			{
				// #TODO : We should log problems to some feed
				CZ_LOG(logMQTTUI, Warning, "Soil moisture reading for pair %d is invalid (%s)", static_cast<int>(e.index), e.reading.getStatusText());
			}
		}
		break;

	}
} 

bool MQTTUI::processCommand(const Command& cmd)
{
	return false;
}

} // namespace cz
