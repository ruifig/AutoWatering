#pragma once

#include "Component.h"
#include "MQTTCache.h"

namespace cz
{

class MQTTUI : public Component, public MQTTCache::Listener
{
  public:

  MQTTUI() = default;
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

	bool m_subscribed = false;

	const MQTTCache::Entry* m_deviceNameValue = nullptr;
};

} // namespace cz

CZ_DECLARE_LOG_CATEGORY(logMQTTUI, Log, Verbose)