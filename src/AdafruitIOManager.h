#pragma once

#include "Config/Config.h"
#include "Component.h"
#include <WiFi.h>
#include <memory>

// This code expects to be compiled for the Pico W only at the moment
#if !( defined(ARDUINO_RASPBERRY_PI_PICO_W) )
	#error For RASPBERRY_PI_PICO_W only
#endif

#if (_ASYNC_MQTT_LOGLEVEL_ > 3)
	#warning Using RASPBERRY_PI_PICO_W with CYC43439 WiFi
#endif

// Forward declarations
class AsyncMqttClient;
enum class AsyncMqttClientDisconnectReason : uint8_t;
struct AsyncMqttClientMessageProperties;

namespace cz
{

/**
 * 
 * 
 * NOTE: The wifi and mqtt conection code was originally based on AsyncMQTT_Generic's "FullyFeatured_RP2040W.ino" example
*/
class AdafruitIOManager : public Component
{
  public:
	AdafruitIOManager();
	~AdafruitIOManager();

	void begin();

  private:
	virtual float tick(float deltaSeconds) override;
	virtual void onEvent(const Event& evt) override;

	void printWifiStatus();
	bool connectToWifi();
	bool isWiFiConnected();
	void connectToMqttLoop();
	void connectToMqtt();
	void printSeparationLine();

	void onMqttConnect(bool sessionPresent);
	static void onMqttConnectCallback(bool sessionPresent);

	void onMqttDisconnect(AsyncMqttClientDisconnectReason reason);
	static void onMqttDisconnectCallback(AsyncMqttClientDisconnectReason reason);

	void onMqttSubscribe(const uint16_t& packetId, const uint8_t& qos);
	static void onMqttSubscribeCallback(const uint16_t& packetId, const uint8_t& qos);

	void onMqttUnsubscribe(const uint16_t& packetId);
	static void onMqttUnsubscribeCallback(const uint16_t& packetId);

	void onMqttMessage(char* topic, char* payload, const AsyncMqttClientMessageProperties& properties, const size_t& len,
					const size_t& index, const size_t& total);
	static void onMqttMessageCallback(char* topic, char* payload, const AsyncMqttClientMessageProperties& properties, const size_t& len,
					const size_t& index, const size_t& total);

	void onMqttPublish(const uint16_t& packetId);
	static void onMqttPublishCallback(const uint16_t& packetId);

	std::unique_ptr<AsyncMqttClient> m_mqttClient;
	int m_status = WL_IDLE_STATUS;
	bool m_connectedWiFi = false;
	bool m_connectedMQTT = false;
	static AdafruitIOManager* ms_instance;
};

} // namespace cz
