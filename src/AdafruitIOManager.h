#pragma once

#include "Config/Config.h"
#include "Component.h"
#include <WiFi.h>
#include <memory>
#include <crazygaze/micromuc/Logging.h>
#include <crazygaze/micromuc/StringUtils.h>
#include <vector>
#include "MQTTCache.h"

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
class AdafruitIOManager : public Component, public MQTTCache::Listener
{
  public:
	AdafruitIOManager();
	~AdafruitIOManager();

	void begin();

	void logCache() const;

  private:
	virtual float tick(float deltaSeconds) override;
	virtual void onEvent(const Event& evt) override;

	//
	// MQTTCache::Listener interface
	//
	virtual void onCacheReceived(const MQTTCache::Entry* entry) override;
	virtual void onCacheSent(const MQTTCache::Entry* entry) override;

	void printWifiStatus();
	bool connectToWifi();
	bool isWiFiConnected();
	void connectToMqttLoop();
	void connectToMqtt();
	void printSeparationLine();

	//
	// @param ignoreCached
	//		If true, it will publish regardless of the previously published value
	//		If false, it will compare against the last sent value, and only publish if different.
	void publish(const char* topic, const char* value, uint8_t qos);

	template<typename T>
	void publish(const char* topic, T value, uint8_t qos)
	{
		if constexpr(std::is_integral_v<T>)
		{
			if constexpr(std::is_signed_v<T>)
				publish(topic, cz::formatString("%d", static_cast<int>(value)), qos);
			else
				publish(topic, cz::formatString("%u", static_cast<unsigned int>(value)), qos);
		}
		else
		{
			// NOTE: We can't simply do a static_assert(false), because that will always fail to compile.
			// The solution is to make the static_assert depend on T, so !std::is_same_v<T,T> does the trick since it depends on T and
			// will always be false
			static_assert(!std::is_same_v<T,T>, "Unexpected/unsupported value type");
		}
	}

	// Publishes a float, with 1 decimal point of precision
	template<int Precision>
	void publish(const char* topic, float value, uint8_t qos)
	{
		if constexpr(Precision==0)
		{
			publish(topic, cz::formatString("%.0f", value), qos);
		}
		else if constexpr(Precision==1)
		{
			publish(topic, cz::formatString("%.1f", value), qos);
		}
		else if constexpr(Precision==2)
		{
			publish(topic, cz::formatString("%.2f", value), qos);
		}
		else if constexpr(Precision==3)
		{
			publish(topic, cz::formatString("%.3f", value), qos);
		}
		else
		{
			// NOTE: For this to work, the static_assert needs to depend on Precision
			static_assert(Precision==0, "Invalid precision specified");
		}
	}

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
	MQTTCache m_cache;
	static AdafruitIOManager* ms_instance;
};

} // namespace cz

CZ_DECLARE_LOG_CATEGORY(logMQTT, Log, Verbose)
