#include "AdafruitIOManager.h"
#include <crazygaze/micromuc/Logging.h>
#include <crazygaze/micromuc/StringUtils.h>

// To be included only in main(), .ino with setup() to avoid `Multiple Definitions` Linker Error
#include <AsyncMqtt_Generic.h>

// Check connection every 1s
#define MQTT_CHECK_INTERVAL_MS 1000

// #define MQTT_HOST         IPAddress(192, 168, 2, 110)
#define MQTT_HOST "io.adafruit.com"
#define MQTT_PORT 1883

namespace
{
	const char* outTopic = ADAFRUIT_IO_USERNAME "/feeds/tests.log";
	const char* inTopic = ADAFRUIT_IO_USERNAME "/feeds/tests.button";
}

namespace cz
{

AdafruitIOManager* AdafruitIOManager::ms_instance = nullptr;

AdafruitIOManager::AdafruitIOManager()
{
	assert(ms_instance==nullptr);
	m_mqttClient = std::make_unique<AsyncMqttClient>();
	ms_instance = this;
}

AdafruitIOManager::~AdafruitIOManager()
{
	ms_instance = nullptr;;
}

void AdafruitIOManager::begin()
{
	CZ_LOG(logDefault, Log, "Starting AdafruitIOManager. BOARD_NAME=%s, AsyncMQTT_Generic Version=%s", BOARD_NAME, ASYNC_MQTT_GENERIC_VERSION);

	connectToWifi();
	m_mqttClient->onConnect(onMqttConnectCallback);
	m_mqttClient->onDisconnect(onMqttDisconnectCallback);
	m_mqttClient->onSubscribe(onMqttSubscribeCallback);
	m_mqttClient->onUnsubscribe(onMqttUnsubscribeCallback);
	m_mqttClient->onMessage(onMqttMessageCallback);
	m_mqttClient->onPublish(onMqttPublishCallback);

	m_mqttClient->setServer(MQTT_HOST, MQTT_PORT);
	m_mqttClient->setClientId("AutoWatering");
	m_mqttClient->setCredentials(ADAFRUIT_IO_USERNAME, ADAFRUIT_IO_KEY);

	connectToMqtt();
}

float AdafruitIOManager::tick(float deltaSeconds)
{
	connectToMqttLoop();
	return MQTT_CHECK_INTERVAL_MS/1000.0f;
}

void AdafruitIOManager::onEvent(const Event& evt)
{
}

void AdafruitIOManager::printWifiStatus()
{
	// print the SSID of the network you're attached to:
	CZ_LOG(logDefault, Log, "Connected to SSID: %s", WiFi.SSID().c_str());

	// print your board's IP address:
	IPAddress ip = WiFi.localIP();
	CZ_LOG(logDefault, Log, "Local IP Address: %s", ip.toString().c_str());

	// print the received signal strength:
	long rssi = WiFi.RSSI();
	CZ_LOG(logDefault, Log, "Signal strenght (RSSI): %l dBm", rssi)
}

bool AdafruitIOManager::connectToWifi()
{
	// check for the WiFi module:
	if (WiFi.status() == WL_NO_MODULE)
	{
		CZ_LOG(logDefault, Fatal, "Communication with WiFi module failed!");

		// don't continue
		while (true)
			;
	}

	CZ_LOG(logDefault, Log, "Connecting to SSID: %s", WIFI_SSID);

	#define MAX_NUM_WIFI_CONNECT_TRIES_PER_LOOP 20

	uint8_t numWiFiConnectTries = 0;

	// attempt to connect to WiFi network
	while ((m_status != WL_CONNECTED) && (numWiFiConnectTries++ < MAX_NUM_WIFI_CONNECT_TRIES_PER_LOOP))
	{
		m_status = WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
		delay(500);
	}

	if (m_status != WL_CONNECTED)
	{
		// Restart for Portenta as something is very wrong
		CZ_LOG(logDefault, Log, "Resetting. Can't connect to any WiFi");
		NVIC_SystemReset();
	}

	printWifiStatus();

	m_connectedWiFi = (m_status == WL_CONNECTED);
	return (m_status == WL_CONNECTED);
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

void AdafruitIOManager::connectToMqttLoop()
{
	if (isWiFiConnected())
	{
		if (!m_connectedMQTT)
		{
			m_mqttClient->connect();
		}

		if (!m_connectedWiFi)
		{
			CZ_LOG(logDefault, Log, "WiFi reconnected");
			m_connectedWiFi = true;
		}
	}
	else
	{
		if (m_connectedWiFi)
		{
			CZ_LOG(logDefault, Log, "WiFi disconnected. Reconnecting");
			m_connectedWiFi = false;
			connectToWifi();
		}
	}
}

void AdafruitIOManager::connectToMqtt()
{
	CZ_LOG(logDefault, Log, "Connecting to MQTT...");
	m_mqttClient->connect();
}

void AdafruitIOManager::printSeparationLine()
{
	CZ_LOG(logDefault, Log, "************************************************");
}

void AdafruitIOManager::onMqttConnect(bool sessionPresent)
{
	CZ_LOG(logDefault, Log, "Connected to MQTT broker: %s, port: %d", MQTT_HOST, MQTT_PORT);

	CZ_LOG(logDefault, Log, "outTopic: %s", outTopic);
	CZ_LOG(logDefault, Log, "inTopic: %s", inTopic);

	m_connectedMQTT = true;

	printSeparationLine();
	CZ_LOG(logDefault, Log, "Session present: %d", static_cast<int>(sessionPresent));

	uint16_t packetIdSub = m_mqttClient->subscribe(inTopic, 2);
	CZ_LOG(logDefault, Log, "Subscribing at QoS 2, packetId: %d", static_cast<int>(packetIdSub));


	// As per Adafruit's IO documentation (https://io.adafruit.com/api/docs/mqtt.html#mqtt-retain), "retain is not
	// really implemented. The special workaround Adafruit IO gives us is a special topic "/get", where if we publish
	// anything, the broker will send back just to this client the most recent value of the feed
	uint16_t packetIdSubGet = m_mqttClient->publish(cz::formatString("%s/get", inTopic), 2, false);
	CZ_LOG(logDefault, Log, "Publish for GET at QoS 2");

	m_mqttClient->publish(outTopic, 0, true, "Hello World from AsyncMQTT_GenericTests");
	CZ_LOG(logDefault, Log, "Publishing at QoS 0");

	uint16_t packetIdPub1 = m_mqttClient->publish(outTopic, 1, true, "Test 2");
	CZ_LOG(logDefault, Log, "Publishing at QoS 1, packetId: %d", static_cast<int>(packetIdPub1));

	uint16_t packetIdPub2 = m_mqttClient->publish(outTopic, 2, true, "Test 3");
	CZ_LOG(logDefault, Log, "Publishing at QoS 2, packetId: %d", static_cast<int>(packetIdPub2));

	printSeparationLine();
}

void AdafruitIOManager::onMqttConnectCallback(bool sessionPresent)
{
	ms_instance->onMqttConnect(sessionPresent);
}

void AdafruitIOManager::onMqttDisconnect(AsyncMqttClientDisconnectReason reason)
{
	(void)reason;
	m_connectedMQTT = false;
	CZ_LOG(logDefault, Warning, "Disconnected from MQTT.");
}

void AdafruitIOManager::onMqttDisconnectCallback(AsyncMqttClientDisconnectReason reason)
{
	ms_instance->onMqttDisconnect(reason);
}

void AdafruitIOManager::onMqttSubscribe(const uint16_t& packetId, const uint8_t& qos)
{
	CZ_LOG(logDefault, Log, "Subscribe acknowledged. packetId=%d, qos=%d", static_cast<int>(packetId), static_cast<int>(qos));
}

void AdafruitIOManager::onMqttSubscribeCallback(const uint16_t& packetId, const uint8_t& qos)
{
	ms_instance->onMqttSubscribe(packetId, qos);
}

void AdafruitIOManager::onMqttUnsubscribe(const uint16_t& packetId)
{
	CZ_LOG(logDefault, Log, "Unsubscribe acknowledged: packetId=%d", static_cast<int>(packetId));
}

void AdafruitIOManager::onMqttUnsubscribeCallback(const uint16_t& packetId)
{
	ms_instance->onMqttUnsubscribe(packetId);
}

void AdafruitIOManager::onMqttMessage(char* topic, char* payload, const AsyncMqttClientMessageProperties& properties, const size_t& len,
                   const size_t& index, const size_t& total)
{
	char message[len + 1];

	memcpy(message, payload, len);
	message[len] = 0;

	CZ_LOG(logDefault, Log, "Publish received: topic=%s, message=\"%s\", qos=%d, dup=%d, retain=%d, len=%u, index=%u, total=%u",
		topic,
		message,
		static_cast<int>(properties.qos),
		static_cast<int>(properties.dup),
		static_cast<int>(properties.retain),
		static_cast<unsigned int>(len),
		static_cast<unsigned int>(index),
		static_cast<unsigned int>(total)
	);

}

void AdafruitIOManager::onMqttMessageCallback(char* topic, char* payload, const AsyncMqttClientMessageProperties& properties, const size_t& len,
                   const size_t& index, const size_t& total)
{
	ms_instance->onMqttMessage(topic, payload, properties, len, index, total);
}

void AdafruitIOManager::onMqttPublish(const uint16_t& packetId)
{
	CZ_LOG(logDefault, Log, "Publish acknowledged: packetId=%d", static_cast<int>(packetId));
}

void AdafruitIOManager::onMqttPublishCallback(const uint16_t& packetId)
{
	ms_instance->onMqttPublish(packetId);
}

} // namespace cz
