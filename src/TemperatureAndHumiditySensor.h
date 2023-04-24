#pragma once

#include <Arduino.h>
#include <assert.h>
#include "Config/Config.h"
#include "Component.h"
#include <Adafruit_HTU21DF.h>

namespace cz
{

/**
 * Temperature and Humidity sensor
 */
class TemperatureAndHumiditySensor : public Component
{
  public:
	TemperatureAndHumiditySensor();

  private:

	//
	// Component interface
	//
	virtual const char* getName() const override { return "TemperatureAndHumiditySensor"; }
	virtual bool initImpl() override;
	virtual float tick(float deltaSeconds) override;
	virtual void onEvent(const Event& evt) override;

	enum class State : uint8_t
	{
		Initializing,
		Idle,
		Reading
	};

	static const char* const ms_stateNames[3];

	virtual void readSensor(float& temperature, float& humidity);
	virtual void resetSensor();

	void changeToState(State newState);
	void onLeaveState();
	void onEnterState();

	float m_timeInState;
	float m_nextTickWait = 0;
	float m_timeSinceLastRead = 0;
	State m_state = State::Initializing;
	Adafruit_HTU21DF m_htu;
};

} // namespace cz

