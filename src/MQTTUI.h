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

	void publishCalibrationInfo();
	void startCalibration(int index);
	void resetCalibration();
	void cancelCalibration();
	void saveCalibration();
	String createConfigJson();
	void publishConfig();
	void publishGroupData(int index);
	bool m_subscribed = false;

	enum State : uint8_t
	{
		WaitingForConnection,
		WaitingForConfig,
		Idle,
		CalibratingSensor
	};

	static const char* const ms_stateNames[4];
	float m_timeInState = 0;
	State m_state = State::WaitingForConnection;

	void changeToState(State newState);
	void onLeaveState();
	void onEnterState();

	void setSubscriptions(bool config, bool group);

	// We need this to detect the first tick after a wifi connected event
	// This is because connecting to the Wifi can take a long time (>10 seconds), and so the first tick will
	// have a high "deltaSeconds" which we want to discard.
	bool m_ignoreNextTick = true;

	// When we receive a configuration change from the MQTT broker, we don't save to EEPROM/Flash straight away, to minimize flash wearing.
	// Every time we receive a change, we set a delay and only save when it reaches 0.
	// This also makes it possible to fiddle with the MQTT UI to adjust values, and only after we stop fiddling it will save the changes.
	float m_saveDelay = 0.0f;

	// Dummy config we act on while calibrating a sensor
	GroupConfig m_dummyCfg;
	// What sensor are we calibrating or -1 if not calibrating any sensor
	int m_calibratingIndex = -1;
};

} // namespace cz

CZ_DECLARE_LOG_CATEGORY(logMQTTUI, Log, Verbose)