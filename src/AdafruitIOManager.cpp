#include "AdafruitIOManager.h"
#include <crazygaze/micromuc/MathUtils.h>

// Check connection every 1s
// #TODO : Is this being used?
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

void AdafruitIOManager::begin()
{
	m_multi.addAP(WIFI_SSID, WIFI_PASSWORD);
	//connectToWifi();

	m_mqtt.system = std::make_unique<MyMqttSystem>();
	m_mqtt.logger = std::make_unique<MyMqttLogger>();
	m_mqtt.network = std::make_unique<MqttClient::NetworkClientImpl<WiFiClient>>(m_wifiClient, *m_mqtt.system);
	m_mqtt.sendBuffer = std::make_unique<MqttClient::ArrayBuffer<512>>();
	m_mqtt.recvBuffer = std::make_unique<MqttClient::ArrayBuffer<1024>>();
	// Allow up to X subscriptions simultaneously
	m_mqtt.messageHandlers = std::make_unique<MqttClient::MessageHandlersImpl<20>>();
	MqttClient::Options mqttOptions;
	mqttOptions.commandTimeoutMs = 10000;
	m_mqtt.client = std::make_unique<MqttClient>(
		mqttOptions, *m_mqtt.logger, *m_mqtt.system, *m_mqtt.network,
		*m_mqtt.sendBuffer, *m_mqtt.recvBuffer, *m_mqtt.messageHandlers);

	m_cache.begin(m_mqtt.client.get(), this, 2.0f);
}

bool AdafruitIOManager::_REMOVEME()
{
	if (connectToWifi())
	{
		return connectToMqtt();
	}
	else
	{
		return false;
	}
}

float AdafruitIOManager::tick(float deltaSeconds)
{
#if 0
	if (connectToWifi())
	{
		connectToMqtt();
	}
#endif

	if (m_mqtt.client->isConnected())
	{
		m_mqtt.client->yield(1);
	}
	else
	{
		CZ_LOG(logDefault, Warning, "MQTT not connected");
	}

	// Tick publishing queue
	m_cache.tick(deltaSeconds, m_mqtt.client->isConnected());

	return MQTT_CHECK_INTERVAL_MS/1000.0f;
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

bool AdafruitIOManager::connectToWifi()
{
	if (m_wifiClient.connected())
	{
		return true;
	}

	#define MAX_NUM_WIFI_CONNECT_TRIES_PER_LOOP 20
	uint8_t numWifiConnectTries = 0;
	while(m_wifiClient.connected() != WL_CONNECTED)
	{
		CZ_LOG(logDefault, Log, "Connecting to %s", WIFI_SSID);
		if (m_multi.run() == WL_CONNECTED)
		{
			CZ_LOG(logDefault, Log, "Wifi connected. IP %s", m_wifiClient.localIP().toString().c_str());
			printWifiStatus();
			return true;
		}

		numWifiConnectTries++;
		if (numWifiConnectTries > MAX_NUM_WIFI_CONNECT_TRIES_PER_LOOP)
		{
			// Restart for Portenta as something is very wrong
			CZ_LOG(logMQTT, Error, "Can't connect to any WiFi. Resetting");
			cz::LogOutput::flush();
			delay(5000);
			NVIC_SystemReset();
			return false;
		}
		else
		{
			delay(500);
		}
	}

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

bool AdafruitIOManager::connectToMqtt()
{
	if (m_mqtt.client->isConnected())
	{
		return true;
	}
	else
	{
		// Close connection if exists
		m_wifiClient.stop();

		// Re-establish TCP connection with MQTT broker
		CZ_LOG(logDefault, Log, "Creating TCP connection to %s:%u...", MQTT_HOST, MQTT_PORT);
		m_wifiClient.connect(MQTT_HOST, MQTT_PORT);
		if (!m_wifiClient.connected())
		{
			CZ_LOG(logDefault, Error, "Can't establish the TCP connection to %s:%d", MQTT_HOST, MQTT_PORT);
			return false;
		}
		else
		{
			CZ_LOG(logDefault, Log, "TCP connection created");
		}

		// Start new MQTT connection
		{
			CZ_LOG(logDefault, Log, "Starting MQTT session...");
			MqttClient::ConnectResult connectResult;
			MQTTPacket_connectData options = MQTTPacket_connectData_initializer;
			options.MQTTVersion = 4;
			options.clientID.cstring = (char *)MQTT_CLIENT_ID;
			options.cleansession = true;
			options.keepAliveInterval = 15; // 15 seconds
			options.username.cstring = ADAFRUIT_IO_USERNAME;
			options.password.cstring = ADAFRUIT_IO_KEY;
			MqttClient::Error::type rc = m_mqtt.client->connect(options, connectResult);
			if (rc != MqttClient::Error::SUCCESS)
			{
				CZ_LOG(logDefault, Error, "MQTT Session start error: %i", rc);
				return false;
			}
			else
			{
				CZ_LOG(logDefault, Log, "MQTT Session started")
			}
		}

		// Subscribe
		{

		}

		return true;
	}
}

void AdafruitIOManager::printSeparationLine()
{
	CZ_LOG(logMQTT, Log, "************************************************");
}

void AdafruitIOManager::onMqttMessage(MqttClient::MessageData& md)
{
	const MqttClient::Message &msg = md.message;
	char payload[msg.payloadLen + 1];
	memcpy(payload, msg.payload, msg.payloadLen);
	payload[msg.payloadLen] = '\0';
	CZ_LOG(logDefault, Log,
		   "Message arrived: qos %d, retained %d, dup %d, packetid %d, payload:[%s]",
		   msg.qos, msg.retained, msg.dup, msg.id, payload);
}

void AdafruitIOManager::onMqttMessageCallback(MqttClient::MessageData& msg)
{
	ms_instance->onMqttMessage(msg);
}

void AdafruitIOManager::logCache() const
{
	m_cache.logState();
}

} // namespace cz
