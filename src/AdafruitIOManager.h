#pragma once

#include "Config/Config.h"
#include "Component.h"
#include <WiFi.h>
#include <crazygaze/micromuc/Logging.h>
#include <crazygaze/micromuc/StringUtils.h>
#include "MQTTCache.h"
#include <memory>

namespace cz
{

/**
 * 
*/
class AdafruitIOManager : public Component, public MQTTCache::Listener
{
  public:
	AdafruitIOManager();
	~AdafruitIOManager();

	void begin();

	void logCache() const;

	bool _REMOVEMEconnectToWifi()
	{
		return connectToWifi();
	}

	bool _REMOVEME();

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
	bool connectToMqtt();
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

	void onMqttMessage(MqttClient::MessageData& msg);
	static void onMqttMessageCallback(MqttClient::MessageData& msg);

	WiFiClient m_wifiClient;
	WiFiMulti m_multi;
	struct MqttObjects
	{
		std::unique_ptr<MqttClient::System> system;
		std::unique_ptr<MqttClient::Logger> logger;
		std::unique_ptr<MqttClient::Network> network;
		std::unique_ptr<MqttClient::Buffer> sendBuffer;
		std::unique_ptr<MqttClient::Buffer> recvBuffer;
		std::unique_ptr<MqttClient::MessageHandlers> messageHandlers;
		std::unique_ptr<MqttClient> client;
	} m_mqtt;

	MQTTCache m_cache;
	static AdafruitIOManager* ms_instance;
};

} // namespace cz

CZ_DECLARE_LOG_CATEGORY(logMQTT, Log, Verbose)
