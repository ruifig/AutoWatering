#pragma once

#include <vector>
#include <queue>
#include <memory>

#include <crazygaze/micromuc/Logging.h>
#include <crazygaze/micromuc/Ticker.h>

#include "Component.h"

#define MQTT_LOG_ENABLED 1
#include "MqttClient.h"
#include "WiFi.h"
#include <ArduinoJson.h>

// When using Adafruit IO, we can get the current value of a feed by publishing to to a special feed (Feed+"/get")
// See https://io.adafruit.com/api/docs/mqtt.html#mqtt-retain
#define USE_ADAFRUITIO_GET 1

// The MQTT library I'm currently using has blocking publish
#define HAS_BLOCKING_PUBLISH 1

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
class MQTTCache : public Component
{
  public:
	MQTTCache();
	~MQTTCache();

	static MQTTCache* getInstance();

	enum class State : uint8_t
	{
		New,
		QueuedForSend, // Marked for publishing. This normally means publish is delayed because of rate limiting
		//QueuedForGet, // This only applies if USE_ADAFRUITIO_GET is set to 1
		SentAndWaitingForAck, // message passed to the mqtt client object, and we are waiting for confirmation the broker got it (if using qos 1 or 2)
		Synced // Synched with the mqtt broker. If the publishing was made with qos 0, then this doesn't necessarily mean the MQTT broker got this current value
	};

	struct Entry
	{
		Entry()
		{
			pendingRemoval = false;
		}
		uint32_t hash = 0;
		String topic;
		String value;
		State state = State::New;
		// When was the last time this entry was synched (In seconds, from when the program started running)
		float lastSyncTime = 0;
		// If a publish is in progress, this contains the packet id
		uint16_t packetId = 0;
		// If a publish is needed or is in progress, this is the desired qos
		uint8_t qos = 1;
		bool pendingRemoval : 1;

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
		float publishInterval = AW_MQTT_PUBLISHINTERVAL;

		int sendBufferSize = 1024*2;
		// The biggest message we need to receive from the MQTT broker is the initial message with the entire group.
		// Therefore we make the recv size a function of AW_MAX_NUM_PAIR. It's not 100% accurate, since there is some other overhead like
		// the feeds that are not part of a sensor/motor pair, but given some slack it should be fine.
		// At the time of writting, 512 bytes for the non-pair fields and 512 bytes per sensor/motor pair seems plenty
		int recvBufferSize = 512 + AW_MAX_NUM_PAIRS * 512;
		int maxNumSubscriptions = 5;

		// Passed to the ArduinoMqtt library's Options.commandTimeoutMs
		unsigned long commandTimeoutMs = 4000;
	};

	struct Listener
	{
		/**
		 * Called when a new message is received, and caused an entry to change value
		 */
		virtual void onMqttValueReceived(const Entry* entry) = 0;

		/**
		 * Called once when the mqtt client confirms an entry as as received by the broker.
		 * If qos 0 was used for sending, this doesn't necessariy mean the broker got it.
		*/
		//virtual void onMqttValueSent(const Entry* entry) = 0;

	};

	/**
	 * This should be called before the Component::initAll to override any of the default options
	*/
	void setOptions(const Options& options);

	void setListener(Listener& listener)
	{
		m_listener = &listener;
	}

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

	const Entry* set(const char* topic, int value, uint8_t qos, bool forceSync = false)
	{
		return set(topic, *IntToString(value), qos, forceSync);
	}

	template<int Precision>
	const Entry* set(const char* topic, float value, uint8_t qos, bool forceSync = false)
	{
		return set(topic, *FloatToString<20,0,Precision>(value), qos, forceSync);
	}

	template<int Precision>
	const Entry* set(const Entry* entry, float value, uint8_t qos, bool forceSync = false)
	{
		return set(entry, *FloatToString<20,0,Precision>(value), qos, forceSync);
	}

#if 0
	// Creates the given entry, but doesn't set any values. It will retrieve the latest value from the MQTT broker
	// If the topic already exists, it simply returns the entry without doing anything
	const Entry* create(const char* topic, uint8_t qos);
#endif

	/**
	 * Subscribes to the given topic (can include wildcards)
	 * A new Entry is created for any feed updates received.
	*/
	void subscribe(const char* topic);

	/**
	 * Finds the specified cache entry.
	 * Returns the entry or NULL if it doesn't exist
	*/
	const Entry* get(const char* topic);

	/**
	 * Removes the specified entry from the cache.
	 * - Removal will be delayed if a publish is in progress.
	 * WARNING: Any pointers to the entry should be considered invalid after this call.
	 */
	void remove(const char* topic);
	void remove(const Entry* entry);
	void logState() const;
	bool isConnected() const;

	//
	// Component interface
	//
	virtual const char* getName() const override { return "MQTTCache"; }
	virtual bool initImpl() override;
	virtual float tick(float deltaSeconds) override;
	virtual void onEvent(const Event& evt) override;
	virtual bool processCommand(const Command& cmd) override;

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

	/**
	 * NOTE: These callbacks were being used with other mqtt libraries I've tried, which actually had async publish.
	 * The MQTT client library I'm using at the moment does blocking publishes, so this is called synchronously
	 * right after the publish
	*/
	void onMqttPublish(uint16_t packetId);
	static void onMqttPublishCallback(uint16_t packetId);
	void onMqttPublish(Entry* entry);

	static MQTTCache* ms_instance;
	std::vector<std::unique_ptr<Entry>> m_cache;
	std::queue<Entry*> m_sendQueue;

	struct Subscription
	{
		String topic;
		bool newSubscription = false;
	};
	std::vector<Subscription> m_subscriptions;
	bool m_hasNewSubscriptions = false;
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

	bool connectToMqttBroker(float deltaSeconds);
	void doSubscribe(const char* topic);
	float m_connectToMqttBrokerCountdown = 0;

	// To avoid reallocating memory every time we receive a message (and possibly reduce fragmentation) we reuse the object
	// See https://arduinojson.org/v6/how-to/reuse-a-json-document/
	std::unique_ptr<DynamicJsonDocument> m_jsondoc;

	// Used to copy over a received payload.
	// Since a payload can be big if there are lots of sensor/motor pairs, I prefer to put in the heap instead of stack
	std::unique_ptr<char[]> m_payloadBuffer;

#if AW_MQTT_WIFI_RECONNECT
	int m_conFailCount = 0;
	bool m_simulateTCPFail = false; 
#endif
};

#if AW_WIFI_ENABLED
	extern MQTTCache gMQTTCache;
#endif

} // namespace cz

CZ_DECLARE_LOG_CATEGORY(logMQTTCache, Log, Verbose)
