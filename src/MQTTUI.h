#pragma once

#include "Component.h"
#include "MQTTCache.h"

namespace cz
{

class MQTTUI : public Component, public MQTTCache::Listener
{
  public:

  MQTTUI();
  virtual ~MQTTUI() = default;

  private:
	// Component interface
	virtual const char* getName() const override { return "MQTTUI"; }
	virtual bool initImpl() override;
	virtual float tick(float deltaSeconds) override;
	virtual void onEvent(const Event& evt) override;
	virtual bool processCommand(const Command& cmd) override;

	// MQTTCache::Listener interface
	virtual void onMqttValueReceived(const MQTTCache::Entry* entry) override;
	//virtual void onMqttValueSent(const MQTTCache::Entry* entry) override {}

	void publishConfig();
	void publishGroupConfig(int index);
	bool m_subscribed = false;

	bool m_configSent = false;

	// We need this to detect the first tick after a wifi connected event
	// This is because connecting to the Wifi can take a long time (>10 seconds), and so the first tick will
	// have a high "deltaSeconds" which we want to discard.
	bool m_firstTick = true;
	float m_waitingForConfigTimeout = 0.0f;

	const MQTTCache::Entry* m_deviceNameValue = nullptr;
};

} // namespace cz

CZ_DECLARE_LOG_CATEGORY(logMQTTUI, Log, Verbose)