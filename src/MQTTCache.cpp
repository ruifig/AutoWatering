#include "MQTTCache.h"
#include <crazygaze/micromuc/FNVHash.h>
#include <crazygaze/micromuc/StringUtils.h>
#include <crazygaze/micromuc/ScopeGuard.h>
#include <crazygaze/micromuc/Profiler.h>

#include "MqttClient.h"
#include "Component.h"
#include "WifiManager.h"

CZ_DEFINE_LOG_CATEGORY(logMQTTCache);

namespace cz
{

#if AW_WIFI_ENABLED
	MQTTCache gMQTTCache;
#endif

namespace
{
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
			CZ_LOG(logMQTTCache, Verbose, "MQTTCLIENT: %s", msg);
		}
	};

	struct MyBuffer: public MqttClient::Buffer
	{
			// Disable copy and move
			MyBuffer(const MyBuffer&) = delete;
			MyBuffer(MyBuffer&&) = delete;
			MyBuffer& operator=(const MyBuffer&) = delete;
			MyBuffer& operator=(MyBuffer&&) = delete;

			MyBuffer(int size)
			{
				m_size = size;
				m_buf = new unsigned char[size];
				CZ_ASSERT(m_buf);
			}
			~MyBuffer()
			{
				delete[] m_buf;
			}
			virtual unsigned char* get() override {return m_buf;}
			virtual int size() const override {return m_size;}
		private:
			unsigned char* m_buf = nullptr;
			int m_size;
	};

	// Copied from ArduinoMQtt's MessageHandlersImpl, and adapted so its not a template
	class MyMessageHandlers: public MqttClient::MessageHandlers {
		public:
			MyMessageHandlers(int handlers_size)
			{
				this->handlers_size = handlers_size;
				handlers = new MqttClient::MessageHandler[handlers_size];
				CZ_ASSERT(handlers);
				
				for (int i = 0; i < size(); ++i) {
						handlers[i] = MqttClient::MessageHandler();
				}
			}

			virtual ~MyMessageHandlers() {}

			int size() const {return handlers_size;}

			bool isFull() const {
				for (int i = 0; i < size(); ++i) {
					if (!handlers[i].isUsed()) {
						return false;
					}
				}
				return true;
			}

			MqttClient::MessageHandler* get() {return handlers;}

			bool set(const char* topic, MqttClient::MessageHandlerCbk handler) {
				bool res = false;
				int emptyIdx = -1;
				// Try to update existing handler with the same topic
				for (int i = 0; i < size(); ++i) {
					if (handlers[i].isUsed()) {
						if (strcmp(handlers[i].topic, topic) == 0) {
							// Replace
							onDeAllocateTopic(handlers[i].topic, i);
							const char *t = onAllocateTopic(topic, i);
							if (!t) {
								return false;
							}
							handlers[i].topic = t;
							handlers[i].cbk = handler;
							res = true;
							break;
						}
					} else if (emptyIdx < 0) {
						// Store empty slot index
						emptyIdx = i;
					}
				}
				// Check result and try to use empty slot if available
				if (!res && emptyIdx >= 0) {
					// Set to the first empty slot
					const char *t = onAllocateTopic(topic, emptyIdx);
					if (!t) {
						return false;
					}
					handlers[emptyIdx].topic = t;
					handlers[emptyIdx].cbk = handler;
					res = true;
				}
				return res;
			}

			void reset(const char* topic) {
				for (int i = 0; i < size(); ++i) {
					if (handlers[i].isUsed() && strcmp(handlers[i].topic, topic) == 0) {
						onDeAllocateTopic(handlers[i].topic, i);
						handlers[i].reset();
						break;
					}
				}
			}

			void reset() {
				for (int i = 0; i < size(); ++i) {
					if (handlers[i].isUsed()) {
						onDeAllocateTopic(handlers[i].topic, i);
						handlers[i].reset();
					}
				}
			}

		protected:
			virtual const char* onAllocateTopic(const char *topic, int storageIdx) {
				// Keep provided pointer
				return topic;
			}

			virtual void onDeAllocateTopic(const char *topic, int storageIdx) {
				// Nothing to do
			}

		private:
			int handlers_size;
			MqttClient::MessageHandler *handlers;
	};
}

MQTTCache* MQTTCache::ms_instance;

MQTTCache::MQTTCache()
{
	CZ_ASSERT(ms_instance == nullptr);
	ms_instance = this;
}

MQTTCache::~MQTTCache()
{
	ms_instance = nullptr;
}

void MQTTCache::setOptions(const MQTTCache::Options& options)
{
	m_cfg.host = options.host;
	m_cfg.port = options.port;
	m_cfg.clientId = options.clientId;
	m_cfg.username = options.username;
	m_cfg.password = options.password;
	m_cfg.publishInterval = options.publishInterval;

	//m_listener = listener;

	m_mqtt.system = std::make_unique<MyMqttSystem>();
	m_mqtt.logger = std::make_unique<MyMqttLogger>();
	m_mqtt.network = std::make_unique<MqttClient::NetworkClientImpl<WiFiClient>>(m_wifiClient, *m_mqtt.system);
	m_mqtt.sendBuffer = std::make_unique<MyBuffer>(options.sendBufferSize);
	m_mqtt.recvBuffer = std::make_unique<MyBuffer>(options.recvBufferSize);
	// Allow up to X subscriptions simultaneously

	m_mqtt.messageHandlers = std::make_unique<MyMessageHandlers>(options.maxNumSubscriptions);
	MqttClient::Options mqttOptions;
	mqttOptions.commandTimeoutMs = options.commandTimeoutMs;
	m_mqtt.client = std::make_unique<MqttClient>(
		mqttOptions, *m_mqtt.logger, *m_mqtt.system, *m_mqtt.network,
		*m_mqtt.sendBuffer, *m_mqtt.recvBuffer, *m_mqtt.messageHandlers);
}

MQTTCache::Entry* MQTTCache::find(const char* topic, bool create, int* index)
{
	uint32_t hash = Hash::fnv_32a_str(topic);

	for (int i = 0; i < m_cache.size(); i++)
	{
		if (m_cache[i]->hash == hash)
		{
			if (index)
				*index = i;
			return m_cache[i].get();
		}
	}

	if (create)
	{
		// Not found in the cache, so create an entry
		auto entry = std::make_unique<MQTTCache::Entry>();
		CZ_ASSERT(entry);
		entry->hash = hash;
		entry->topic = topic;
		CZ_LOG(logMQTTCache, Log, "find:create: %s", toLogString(entry.get()));
		m_cache.push_back(std::move(entry));
		if (index)
			*index = m_cache.size() -1;
		return m_cache.back().get();
	}
	else
	{
		return nullptr;
	}
}

MQTTCache::Entry* MQTTCache::findByPacketId(uint16_t packetId, int* index)
{
	for (int i = 0; i < m_cache.size(); i++)
	{
		if (m_cache[i]->packetId == packetId)
		{
			if (index)
				*index = i;
			return m_cache[i].get();
		}
	}

	CZ_LOG(logMQTTCache, Log, "findByPacketId: packetId %u not found", static_cast<unsigned int>(packetId));
	return nullptr;
}


const MQTTCache::Entry* MQTTCache::set(const char* topic, const char* value, uint8_t qos, bool forceSync)
{
	return set(find(topic, true), value, qos, forceSync);
}

const MQTTCache::Entry* MQTTCache::set(const MQTTCache::Entry* inEntry, const char* value, uint8_t qos, bool forceSync)
{
	CZ_LOG(logMQTTCache, Log, "set: (%s), value='%s', qos=%d, forceSync=%d", toLogString(inEntry), value, static_cast<int>(qos), static_cast<int>(forceSync));

	auto entry = const_cast<MQTTCache::Entry*>(inEntry);
	// If this entry was pending for removal, we consider it as part of the cache again and never delete it
	entry->pendingRemoval = false;

	// Set the qos for any future publishings. As-in, if this entry is already pending for sending and this call 
	// will not trigger anything because the value is already the same, we still want to change the qos.
	entry->qos = qos;

	if (entry->state == MQTTCache::State::New || entry->value != value || forceSync)
	{
		entry->value = value;
		// Queue for send if not queued already
		if (entry->state != MQTTCache::State::QueuedForSend)
		{
			// Mark this as 0, so if we receive a publish ack due to some other update, it gets ignored
			entry->packetId = 0;
			entry->state = MQTTCache::State::QueuedForSend;
			CZ_LOG(logMQTTCache, Log, "set: Queued for sending");
			m_sendQueue.push(entry);
		}
	}

	return entry;
}

const MQTTCache::Entry* MQTTCache::get(const char* topic)
{
	return find(topic, false);
}

void MQTTCache::remove(const char* topic)
{
	int index;
	if (MQTTCache::Entry* entry = find(topic, false, &index))
	{
		doRemove(index);
	}
}

void MQTTCache::remove(const MQTTCache::Entry* entry)
{
	for(int i=0; i<m_cache.size(); i++)
	{
		if (m_cache[i].get() == entry)
		{
			doRemove(i);
		}
	}
}

void MQTTCache::doRemove(int index)
{
	MQTTCache::Entry* e = m_cache[index].get();
	CZ_LOG(logMQTTCache, Log, "doRemove:%d: %s", index, toLogString(e));

	if (e->isUpdating())
	{
		e->pendingRemoval = true;
		CZ_LOG(logMQTTCache, Log, "doRemove: Pending removal");
	}
	else
	{
		CZ_LOG(logMQTTCache, Log, "doRemove: Removed");
		m_cache.erase(m_cache.begin() + index);
	}
}

void MQTTCache::onMqttMessage(MqttClient::MessageData& md)
{
	const MqttClient::Message &msg = md.message;
	char payload[msg.payloadLen + 1];
	memcpy(payload, msg.payload, msg.payloadLen);
	payload[msg.payloadLen] = '\0';
	CZ_LOG(logMQTTCache, Log,
		   "Message arrived: qos %d, retained %d, dup %d, packetid %d, payload:[%s]",
		   msg.qos, msg.retained, msg.dup, msg.id, payload);

	const char* topic = md.topicName.cstring;
	auto entry = find(topic, false);

	// We only accept entries that are registered
	if (!entry)
	{
		CZ_LOG(logMQTTCache, Log, "onMqttMessage: '%s'. Ignoring because not present in cache", topic);
		return;
	}

	CZ_LOG(logMQTTCache, Log, "onMqttMessage: (%s), message='%s'", toLogString(entry), payload);

	// If we have a value in the cache queued up for sending, then it's easier to just ignore the message
	// This means the cache will keep the value we have queued up for sending
	if (entry->isUpdating())
	{
		CZ_LOG(logMQTTCache, Log, "onMqttMessage: Ignoring message because entry is in update status");
		return;
	}

	entry->state = MQTTCache::State::Synced;
	if (entry->value != payload)
	{
		entry->value = payload;
		CZ_LOG(logMQTTCache, Log, "onCacheReceived: %s", toLogString(entry));
		//m_listener->onCacheReceived(entry);
	}
}

void MQTTCache::onMqttMessageCallback(MqttClient::MessageData& md)
{
	CZ_ASSERT(ms_instance);
	ms_instance->onMqttMessage(md);
}

void MQTTCache::onMqttPublish(uint16_t packetId)
{
	CZ_LOG(logMQTTCache, Log, "onMqttPublish: packetId=%u", static_cast<unsigned int>(packetId));
	int index;
	auto entry = findByPacketId(packetId, &index);
	if (!entry)
	{
		CZ_LOG(logMQTTCache, Log, "onMqttPublish: No entry found. Ignoring.");
		return;
	}

	CZ_LOG(logMQTTCache, Log, "onMqttPublish: %s", toLogString(entry));

	CZ_ASSERT(entry->state == MQTTCache::State::SentAndWaitingForAck);
	entry->state == MQTTCache::State::Synced;

	CZ_LOG(logMQTTCache, Log, "onCacheSent: %s", toLogString(entry));
	//m_listener->onCacheSent(entry);

	entry->packetId = 0;
	if (entry->pendingRemoval)
	{
		doRemove(index);
		return;
	}
}

void MQTTCache::onMqttPublishCallback(uint16_t packetId)
{
	CZ_ASSERT(ms_instance);
	ms_instance->onMqttPublish(packetId);
}

bool MQTTCache::initImpl()
{
	// Set default options, if not set yet
	if (m_cfg.host.length()==0)
	{
		Options options;
		setOptions(options);
	}

	return true;
}

float MQTTCache::tick(float deltaSeconds)
{
	PROFILE_SCOPE(F("MQTTCache"));

	constexpr float tickInterval = 0.25f;
	if (!isConnected())
	{
		if (!connectToMqttBroker(deltaSeconds))
		{
			return tickInterval;
		}
	}

	m_mqtt.client->yield(1);

	m_publishCountdown -= deltaSeconds;
	if (m_publishCountdown > 0 || m_sendQueue.size() == 0)
	{
		return tickInterval;
	}

	auto entry = m_sendQueue.front();
	m_sendQueue.pop();
	CZ_LOG(logMQTTCache, Log, "tick:publish(BEFORE): %s", toLogString(entry));

	// #TODO : Remove this
	CZ_LOG(logMQTTCache, Log,"HEAP INFO: size=%d, used=%d, free=%d", rp2040.getTotalHeap(), rp2040.getUsedHeap(), rp2040.getFreeHeap());

	CZ_ASSERT(entry->state == MQTTCache::State::QueuedForSend);
	CZ_LOG(logMQTTCache, Log, "Publishing...");
	AW_CUSTOM_SERIAL.flush();

	{
		//entry->packetId = m_mqttClient->publish(entry->topic.c_str(), entry->qos, true, entry->value.c_str());
		MqttClient::Message msg;
		msg.qos = static_cast<MqttClient::QoS>(entry->qos);
		msg.retained = true;
		msg.dup = false;
		msg.payload = (void*) entry->value.c_str();
		msg.payloadLen = entry->value.length();
		auto startPublish = millis();
		MqttClient::Error::type rc = m_mqtt.client->publish(entry->topic.c_str(), msg);
		auto endPublish = millis();
		if (rc != MqttClient::Error::SUCCESS)
		{
			CZ_LOG(logMQTTCache, Error, "Failed to publish: %i", rc);
		}
		else
		{
			CZ_LOG(logMQTTCache, Log, "Publish time: %u ms", endPublish - startPublish);
		}
	}

	CZ_LOG(logMQTTCache, Log, "... done");
	AW_CUSTOM_SERIAL.flush();
	// If sending with qos 0, we are not receiving any confirmation, so we set the state to Synced
	entry->state = entry->qos == 0 ? MQTTCache::State::Synced : MQTTCache::State::SentAndWaitingForAck;
	CZ_LOG(logMQTTCache, Log, "tick:publish(AFTER): %s", toLogString(entry));
	m_publishCountdown = m_cfg.publishInterval;
	return tickInterval;
}

void MQTTCache::onEvent(const Event& evt)
{
	switch(evt.type)
	{
		case Event::Type::WifiStatus:
		{
			auto&& e = static_cast<const WifiStatusEvent&>(evt);
			//
		}
		break;
	}
}

bool MQTTCache::processCommand(const Command& cmd)
{
	if (cmd.is("log"))
	{
		logState();
	}
	else if(cmd.is("testtcpfail"))
	{
		if (!cmd.parseParams(m_simulateTCPFail))
		{
			return false;
		}

		if (m_simulateTCPFail)
		{
			m_wifiClient.stop();
		}
	}
	else
	{
		return false;
	}

	return true;
}

const char* MQTTCache::toLogString(const MQTTCache::Entry* e) const
{
	return formatString("hash=%u, topic='%s', value='%s', state=%d, packetId=%u, qos=%d, pendingRemoval=%d",
		e->hash,
		e->topic.c_str(),
		e->value.c_str(),
		static_cast<int>(e->state),
		static_cast<unsigned int>(e->packetId),
		static_cast<int>(e->qos),
		static_cast<int>(e->pendingRemoval)
		);
}

void MQTTCache::logState() const
{
	CZ_LOG(logMQTTCache, Log, "Cached values: %u. Send queue=%u", static_cast<unsigned int>(m_cache.size()), static_cast<unsigned int>(m_sendQueue.size()));
	for(auto&& e : m_cache)
	{
		CZ_LOG(logMQTTCache, Log, "    %s", toLogString(e.get()));
	}
}

bool MQTTCache::isConnected() const
{
	return m_mqtt.client->isConnected();
}

bool MQTTCache::connectToMqttBroker(float deltaSeconds)
{
	if (isConnected() && gWifiManager.isConnected())
	{
		return true;
	}
	else
	{
		m_connectToMqttBrokerCountdown -= deltaSeconds;
		if (m_connectToMqttBrokerCountdown > 0)
		{
			return false;
		}
		m_connectToMqttBrokerCountdown = AW_MQTT_CONNECTION_RETRY_INTERVAL;

		if (!gWifiManager.isConnected())
		{
			CZ_LOG(logMQTTCache, Error, "Can't connect to MQTT broker because WiFi is not connected");
			return false;
		}
		
		auto incrementFailCount = [this]
		{
			#if AW_MQTT_WIFI_RECONNECT
				m_conFailCount++;
				if (m_conFailCount >= AW_MQTT_WIFI_RECONNECT)
				{
					m_conFailCount = 0;
					m_simulateTCPFail = false;
					CZ_LOG(logMQTTCache, Log, "Too many connection attempts. Disconnecting/reconnecting wifi to try and fix it");
					// NOTE: The reconnect is done in AdafruitIOManager once it detects Wifi has disconnected
					gWifiManager.disconnect(true);
				}
			#endif
		};

		// Close connection if exists
		m_wifiClient.stop();

		// Re-establish TCP connection with MQTT broker
		const char* host = m_simulateTCPFail ? "hopefully-this-url-doesnt-exist.com" : m_cfg.host.c_str();
		CZ_LOG(logMQTTCache, Log, "Creating TCP connection to %s:%u...", host , static_cast<unsigned int>(m_cfg.port));
		m_wifiClient.connect(host, m_cfg.port);
		if (!m_wifiClient.connected())
		{
			CZ_LOG(logMQTTCache, Error, "Can't establish the TCP connection to %s:%d", host, static_cast<unsigned int>(m_cfg.port));
			incrementFailCount();
			return false;
		}
		else
		{
			CZ_LOG(logMQTTCache, Log, "TCP connection created");
		}

		// Start new MQTT connection
		{
			CZ_LOG(logMQTTCache, Log, "Starting MQTT session...");
			MqttClient::ConnectResult connectResult;
			MQTTPacket_connectData options = MQTTPacket_connectData_initializer;
			options.MQTTVersion = 4;
			options.clientID.cstring = const_cast<char*>(m_cfg.clientId.c_str());
			options.cleansession = true;
			options.keepAliveInterval = 15; // 15 seconds
			options.username.cstring = const_cast<char*>(m_cfg.username.c_str());
			options.password.cstring = const_cast<char*>(m_cfg.password.c_str());
			MqttClient::Error::type rc = m_mqtt.client->connect(options, connectResult);
			if (rc != MqttClient::Error::SUCCESS)
			{
				CZ_LOG(logMQTTCache, Error, "MQTT Session start error: %i", rc);
				incrementFailCount();
				return false;
			}
			else
			{
				CZ_LOG(logMQTTCache, Log, "MQTT Session started")
			}
		}

		// Subscribe
		{
			// #TODO : Add/Re-add subscriptions
		}

		return true;
	}
}



}