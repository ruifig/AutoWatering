#include "MQTTCache.h"
#include <crazygaze/micromuc/FNVHash.h>
#include <crazygaze/micromuc/StringUtils.h>

#include "MqttClient.h"

CZ_DEFINE_LOG_CATEGORY(logMQTTCache);

namespace cz
{

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

void MQTTCache::begin(MqttClient* mqttClient, MQTTCache::Listener* listener, float publishInterval)
{
	m_mqttClient = mqttClient;
	m_listener = listener;
	m_publishInterval = publishInterval;
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
	const char* topic = md.topicName.cstring;
	auto entry = find(topic, false);

	// We only accept entries that are registered
	if (!entry)
	{
		CZ_LOG(logMQTTCache, Log, "onMqttMessage: '%s'. Ignoring because not present in cache", topic);
		return;
	}


	const int len = md.message.payloadLen;
	char message[len + 1];
	memcpy(message, md.message.payload, len);
	message[len] = 0;

	CZ_LOG(logMQTTCache, Log, "onMqttMessage: (%s), message='%s'", toLogString(entry), message);

	// If we have a value in the cache queued up for sending, then it's easier to just ignore the message
	// This means the cache will keep the value we have queued up for sending
	if (entry->isUpdating())
	{
		CZ_LOG(logMQTTCache, Log, "onMqttMessage: Ignoring message because entry is in update status");
		return;
	}

	entry->state = MQTTCache::State::Synced;
	if (entry->value != message)
	{
		entry->value = message;
		CZ_LOG(logMQTTCache, Log, "onCacheReceived: %s", toLogString(entry));
		m_listener->onCacheReceived(entry);
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
	m_listener->onCacheSent(entry);

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

void MQTTCache::tick(float deltaSeconds, bool checkSendQueue)
{
	m_publishCountdown -= deltaSeconds;
	if (!checkSendQueue || m_publishCountdown > 0 || m_sendQueue.size() == 0)
	{
		return;
	}

	auto entry = m_sendQueue.front();
	m_sendQueue.pop();
	CZ_LOG(logMQTTCache, Log, "tick:publish(BEFORE): %s", toLogString(entry));

	// #TODO : Remove this
	CZ_LOG(logMQTTCache, Log,"HEAP INFO: size=%d, used=%d, free=%d", rp2040.getTotalHeap(), rp2040.getUsedHeap(), rp2040.getFreeHeap());

	CZ_ASSERT(entry->state == MQTTCache::State::QueuedForSend);
	CZ_LOG(logMQTTCache, Log, "Publishing...");
	MySerial.flush();

	{
		//entry->packetId = m_mqttClient->publish(entry->topic.c_str(), entry->qos, true, entry->value.c_str());
		MqttClient::Message msg;
		msg.qos = static_cast<MqttClient::QoS>(entry->qos);
		msg.retained = true;
		msg.dup = false;
		msg.payload = (void*) entry->value.c_str();
		msg.payloadLen = entry->value.length();
		auto startPublish = millis();
		MqttClient::Error::type rc = m_mqttClient->publish(entry->topic.c_str(), msg);
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
	MySerial.flush();
	// If sending with qos 0, we are not receiving any confirmation, so we set the state to Synced
	entry->state = entry->qos == 0 ? MQTTCache::State::Synced : MQTTCache::State::SentAndWaitingForAck;
	CZ_LOG(logMQTTCache, Log, "tick:publish(AFTER): %s", toLogString(entry));
	m_publishCountdown = m_publishInterval;
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


}