#include "AdafruitIOManager.h"
#include "Watchdog.h"
#include <crazygaze/micromuc/MathUtils.h>

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

	class MyMqttSystem : public MqttClient::System
	{
	public:
		virtual unsigned long millis() const override {
			return ::millis();
		}
	};

	class MyMqttLogger : public MqttClient::Logger
	{
	public:

		virtual void println(const char* msg) override
		{
			CZ_LOG(logMQTT, Log, "MQTTCLIENT: %s", msg);
		}
	};

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
	m_multi.addAP(WIFI_SSID, WIFI_PASSWORD);

	MQTTCache::Options options;
	m_cache.begin(options, this);
	return true;
}

float AdafruitIOManager::tick(float deltaSeconds)
{
	connectToWifi(true);

	// Tick publishing queue
	m_cache.tick(deltaSeconds);

	return 0.250f;
}

void AdafruitIOManager::publish(const char* topic, const char* value, uint8_t qos)
{
	m_cache.set(topic, value, qos, true);
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
	if (cmd.is("wifidisconnect"))
	{
		WiFi.disconnect();
	}
	else
	{
		return m_cache.processCommand(cmd);
	}
	
	return true;
}

void AdafruitIOManager::onCacheReceived(const MQTTCache::Entry* entry)
{
}

void AdafruitIOManager::onCacheSent(const MQTTCache::Entry* entry)
{
}

void AdafruitIOManager::printWifiStatus()
{
	// print the SSID of the network you're attached to:
	CZ_LOG(logMQTT, Log, "Connected to SSID: %s", WiFi.SSID().c_str());

	// print your board's IP address:
	IPAddress ip = WiFi.localIP();
	CZ_LOG(logMQTT, Log, "Local IP Address: %s", ip.toString().c_str());

	// print the received signal strength:
	int rssi = WiFi.RSSI();
	CZ_LOG(logMQTT, Log, "Signal strenght (RSSI): %d dBm", rssi)
}

bool AdafruitIOManager::connectToWifi(bool systemResetOnFail)
{
	if (WiFi.status() == WL_CONNECTED)
	{
		return true;
	}

	#define MAX_NUM_WIFI_CONNECT_TRIES_PER_LOOP 20
	int numTries = 0;
	while(WiFi.status() != WL_CONNECTED)
	{
		numTries++;
		CZ_LOG(logMQTT, Log, "Connecting to %s (Attempt %d)", WIFI_SSID, numTries);

		// Disable the watchdog ONLY for as long as we need
		uint8_t runRes;
		{
			WatchdogPauseScope wtdPause;
			runRes = m_multi.run();
		}

		if (runRes == WL_CONNECTED)
		{
			//CZ_LOG(logMQTT, Log, "Wifi connected. IP %s", m_wifiClient.localIP().toString().c_str());
			printWifiStatus();
			break;
		}

		if (numTries > MAX_NUM_WIFI_CONNECT_TRIES_PER_LOOP)
		{
			Component::raiseEvent(WifiEvent(false));
			// Restart for Portenta as something is very wrong
			CZ_LOG(logMQTT, Error, "Can't connect to any WiFi. Resetting");
			cz::LogOutput::flush();
			delay(1000);
			rp2040.reboot();
			return false;
		}
		else
		{
			delay(200);
		}
	}

	Component::raiseEvent(WifiEvent(true));
	return true;
}

bool AdafruitIOManager::isWiFiConnected()
{
	// You can change longer or shorter depending on your network response
	// Shorter => more responsive, but more ping traffic
	static uint8_t theTTL = 10;

	// Use ping() to test TCP connections
	if (WiFi.ping(WiFi.gatewayIP(), theTTL) == theTTL)
	{
		return true;
	}

	return false;
}

#if WIFI_ENABLED
AdafruitIOManager gAdafruitIOManager;
#endif

} // namespace cz
