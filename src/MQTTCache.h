#pragma once

#include <vector>
#include <queue>
#include <memory>

#include "Config/Config.h"
#include <crazygaze/micromuc/Logging.h>

#define MQTT_LOG_ENABLED 1
#include "MqttClient.h"
#include "WiFi.h"

namespace cz
{

/**
 * Local cache for whatever is sent or received from the MQTT broker.
 * This does a couple of things:
 *  - Implements publish rate limiting
 *	- Only publishes when there is a change to the value
 *
 * Any Entry objects passed to the user are guaranteed to exist until until a call to remove is made.
 */
class MQTTCache
{
  public:
	MQTTCache();
	~MQTTCache();

	enum class State : uint8_t
	{
		New,
		QueuedForSend, // Marked for publishing. This normally means publish is delayed because of rate limiting
		SentAndWaitingForAck, // message passed to the mqtt client object, and we are waiting for confirmation the broker got it (if using qos 1 or 2)
		Synced // Synched with the mqtt broker. If the publishing was made with qos 0, then this doesn't necessarily mean the MQTT broker got this current value
	};

	struct Entry
	{
		uint32_t hash = 0;
		String topic;
		String value;
		State state = State::New;
		// If a publish is in progress, this contains the packet id
		uint16_t packetId = 0;
		// If a publish is needed or is in progress, this is the desired qos
		uint8_t qos = 1;
		bool pendingRemoval = false;

		bool isUpdating() const
		{
			return (state == State::QueuedForSend || state == State::SentAndWaitingForAck) ? true : false;
		}
	};

	struct Options
	{
		const char* host = "io.adafruit.com";
		uint16_t port = 1883;
		const char* clientId = "MQTTCache";

		const char* username = ADAFRUIT_IO_USERNAME;
		const char* password = ADAFRUIT_IO_KEY;

		/**
		 * Interval in seconds between publishes.
		 * In short, this implements a simple rate limiting.
		 * NOTE: A free Adafruit IO account allows 30 publishes per minute (1 publish every 2 seconds) PER ACCOUNT, not per device, so make sure you adjust the publishInterval accordingly if you are using multiple devices.
		*/
		float publishInterval = 2.0f;

		int sendBufferSize = 1024;
		int recvBufferSize = 1024;
		int maxNumSubscriptions = 5;

		// Passed to the ArduinoMqtt library's Options.commandTimeoutMs
		unsigned long commandTimeoutMs = 4000;
	};

	struct Listener
	{
		/**
		 * Called when a new message is received, and caused an entry to change value
		 */
		virtual void onCacheReceived(const Entry* entry) = 0;

		/**
		 * Called once when the mqtt client confirms an entry as as received by the broker.
		 * If qos 0 was used for sending, this doesn't necessariy mean the broker got it.
		*/
		virtual void onCacheSent(const Entry* entry) = 0;

	};

	/**
	 * @param mqttClient MQTT client to use
	 * @param listener Listener object
	 * @param publishInterval How long to wait (in seconds) between sends. This is for rate limiting
	 */
	void begin(const Options& options, Listener* listener);

	/**
	 * Sets a cache entry to the specified value.
	 * This triggers a publish if required, such as if the value changed or it's a new entry
	 * @param forceSync
	 * 		Forces a mqtt publish regardless if the value is not changing. Note that it still obeys the rate limiting, so multiple consecutive calls with
	 * 		forceSync set to true doesn't not necessarily mean multiple values are sent to the broker.
	 * NOTE:
	 * Due to rate limiting, a call to set doesn't do a publish straight away. the cache entry is marked for sending (if required).
	 * While the cache entry is marked for sending, multiple calls to set are still allowed, and at the time of send, it uses the lastest value.
	 * 
	 */
	const Entry* set(const char* topic, const char* value, uint8_t qos, bool forceSync = false);
	const Entry* set(const Entry* entry, const char* value, uint8_t qos, bool forceSync = false);

	/**
	 * Removes the specified entry from the cache.
	 * - Removal will be delayed if a publish is in progress.
	 * WARNING: Any pointers to the entry should be considered invalid after this call.
	 */
	void remove(const char* topic);
	void remove(const Entry* entry);

	void tick(float deltaSeconds);

	void logState() const;

	bool isConnected() const;

  private:

	void doRemove(int index);
	const char* toLogString(const MQTTCache::Entry* entry) const;

	/**
	 * @param index If specified, on exit it will contain the entry index
	*/
	Entry* find(const char* topic, bool create, int* index = nullptr);
	Entry* findByPacketId(uint16_t packetId, int* index = nullptr);

	void onMqttMessage(MqttClient::MessageData& md);
	static void onMqttMessageCallback(MqttClient::MessageData& md);

	void onMqttPublish(uint16_t packetId);
	static void onMqttPublishCallback(uint16_t packetId);

	static MQTTCache* ms_instance;
	std::vector<std::unique_ptr<Entry>> m_cache;
	std::queue<Entry*> m_sendQueue;
	float m_publishCountdown = 0;
	Listener* m_listener;
	WiFiClient m_wifiClient;

	struct Config
	{
		String host;
		uint16_t port;
		String clientId;
		String username;
		String password;
		float publishInterval;
	} m_cfg;

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

	bool connectToMqttBroker();
};

} // namespace cz

CZ_DECLARE_LOG_CATEGORY(logMQTTCache, Log, Verbose)
