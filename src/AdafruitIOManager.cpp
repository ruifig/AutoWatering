#include "AdafruitIOManager.h"
#include "Watchdog.h"
#include <crazygaze/micromuc/MathUtils.h>
#include "MQTTCache.h"

#define MQTT_CHECK_INTERVAL_MS 1000

#define MQTT_HOST "io.adafruit.com"
#define MQTT_PORT 1883
#define MQTT_CLIENT_ID "AutoWatering"

CZ_DEFINE_LOG_CATEGORY(logMQTT);

namespace
{
	const char* outTopic = ADAFRUIT_IO_USERNAME "/feeds/tests.log";
	const char* inTopic = ADAFRUIT_IO_USERNAME "/feeds/tests.button";

	#define TOPIC_GROUP "autowatering1."
	namespace Topics
	{
		const char* temperature = ADAFRUIT_IO_USERNAME "/feeds/" TOPIC_GROUP "temperature";
		const char* humidity = ADAFRUIT_IO_USERNAME "/feeds/" TOPIC_GROUP "humidity";

		// @param index Motor/Sensor pair
		const char* moistureLevel(int index)
		{
			return cz::formatString(ADAFRUIT_IO_USERNAME "/feeds/" TOPIC_GROUP "pair%d-moisturelevel", index);
		}
		
		const char* moistureThreshold(int index)
		{
			return cz::formatString(ADAFRUIT_IO_USERNAME "/feeds/" TOPIC_GROUP "pair%d-moisturethreshold", index);
		}
	}
}

namespace cz
{

AdafruitIOManager* AdafruitIOManager::ms_instance = nullptr;

AdafruitIOManager::AdafruitIOManager()
{
	CZ_ASSERT(ms_instance==nullptr);
	ms_instance = this;
}

AdafruitIOManager::~AdafruitIOManager()
{
	ms_instance = nullptr;;
}

bool AdafruitIOManager::initImpl()
{
	return true;
}

float AdafruitIOManager::tick(float deltaSeconds)
{
	return 0.250f;
}

void AdafruitIOManager::publish(const char* topic, const char* value, uint8_t qos)
{
	gMQTTCache.set(topic, value, qos, true);
}

void AdafruitIOManager::onEvent(const Event& evt)
{
	switch(evt.type)
	{
		case Event::SoilMoistureSensorReading:
		{
			auto&& e = static_cast<const SoilMoistureSensorReadingEvent&>(evt);
			const GroupData& data = gCtx.data.getGroupData(e.index);
			if (e.reading.isValid())
			{
				uint8_t perc = static_cast<uint8_t>(data.getCurrentValueAsPercentage());
				publish(Topics::moistureLevel(e.index), data.getCurrentValueAsPercentage(), 2);
				publish(Topics::moistureThreshold(e.index), data.getThresholdValueAsPercentage(), 2);
			}
			else
			{
				// #TODO : We should log problems to some feed
				CZ_LOG(logMQTT, Warning, "Soil moisture reading for pair %d is invalid (%s)", static_cast<int>(e.index), e.reading.getStatusText());
			}
		}
		break;

		case Event::TemperatureSensorReading:
		{
			auto&& e = static_cast<const TemperatureSensorReadingEvent&>(evt);
			publish<1>(Topics::temperature, e.temperatureC, 2);
		}
		break;

		case Event::HumiditySensorReading:
		{
			auto&& e = static_cast<const HumiditySensorReadingEvent&>(evt);
			publish<1>(Topics::humidity, e.humidity, 2);
		}
		break;

		default:
		break;
	}
}

bool AdafruitIOManager::processCommand(const Command& cmd)
{
	return false;
}

#if WIFI_ENABLED
AdafruitIOManager gAdafruitIOManager;
#endif

} // namespace cz
