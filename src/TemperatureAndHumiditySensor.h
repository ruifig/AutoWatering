#pragma once

#include <Arduino.h>
#include <assert.h>
#include "Config.h"
#include "Component.h"

namespace cz
{

/**
 * Temperature and Humidity sensor
 */
class TemperatureAndHumiditySensor : public Component
{
  public:
	TemperatureAndHumiditySensor(IOExpanderPin vinPin, MultiplexerPin dataPin);

	void begin();

	//
	// Component interface
	//
	virtual float tick(float deltaSeconds) override;
	virtual void onEvent(const Event& evt) override;

  private:

	enum class State : uint8_t
	{
		Initializing,
		PoweredDown,
		Reading
	};

	static const char* const ms_stateNames[3];

	virtual void readSensor(float& temperature, float& humidity);

	void changeToState(State newState);
	void onLeaveState();
	void onEnterState();
	bool tryEnterReadingState();

	IOExpanderPin m_vinPin;
	MultiplexerPin m_dataPin;
	float m_timeInState;
	float m_nextTickWait = 0;
	float m_timeSinceLastRead = 0;
	State m_state = State::Initializing;
};

} // namespace cz

