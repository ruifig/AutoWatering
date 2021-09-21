#pragma once

#include <Arduino.h>
#include <assert.h>
#include "Context.h"
#include "Component.h"

namespace cz
{

/**
 * Capacitive Soil Moisture Sensor: https://www.amazon.co.uk/gp/product/B08GC5KT4T
 *
 * These sensor can have a problem where their response to change is very slow. The following links explain how to fix
 * it: https://forum.arduino.cc/index.php?topic=597585.0 https://www.youtube.com/watch?v=QGCrtXf8YSs
 *
 * Setup:
 * - The sensor Vin should be connected to an Arduino digital pin. This is required so that the sensor is only powered
 * up when we need to make a reading. This saves power.
 */
class SoilMoistureSensor : public Component
{
  public:

	/**
	 * @param ctx program context
	 * @param index Sensor's index within the program data
	 * @param vinPin What io expander pin we are using to power this sensor
	 * @param dataPin What multiplexer pin we are using to read the sensor
	 */
	SoilMoistureSensor(Context& ctx, uint8_t index, IOExpanderPin vinPin, MultiplexerPin dataPin);

	// Disable copying
	SoilMoistureSensor(const SoilMoistureSensor&) = delete;
	const SoilMoistureSensor& operator=(const SoilMoistureSensor&) = delete;

	virtual void begin();
	virtual float tick(float deltaSeconds) override;
	virtual void onEvent(const Event& evt) override;

  protected:
	enum class State : uint8_t
	{
		Initializing,
		PoweredDown,
		PoweringUp,
		Reading
	};

#if CZ_LOG_ENABLED
	static const char* const ms_stateNames[4];
#endif

	Context& m_ctx;
	float m_timeInState = 0;
	float m_nextTickWait = 0;
	State m_state = State::Initializing;
	uint8_t m_index;
	IOExpanderPin m_vinPin;
	MultiplexerPin m_dataPin;

	virtual int readSensor();

	void changeToState(State newState);
	void onLeaveState();
	void onEnterState();
	bool tryEnterReadingState();
};

class MockSoilMoistureSensor : public SoilMoistureSensor
{
public:
	using SoilMoistureSensor::SoilMoistureSensor;

	virtual void begin() override;
	virtual float tick(float deltaSeconds) override;
	virtual void onEvent(const Event& evt) override;

protected:

	virtual int readSensor() override;

	static constexpr int ms_delayChaseValue = 5;

	struct
	{
		int dryValue;
		int waterValue;

		// reported value lags behind target value a bits, so we can simulate response time.
		// The sensor won't report moisture as soon as the motor turns on
		float currentValue;
		float targetValue;

		// How fast the current value chases the target value
		float currentValueChaseRate = 8.0f;
		// How fast the target value changes when the motor is on
		float targetValueOnRate = 10.0f;
		// How fast the target value changes when the motor is off
		float targetValueOffRate = 1.0f;

		// How long in seconds to wait for the currentValue to start chasing the target value.
		// This is useful to simulate the fact that once the motor is turned on, it might take a bit for the soil
		// to soak the water and for the sensor to react.

		float currentValueChaseDelay = 0;

		bool motorIsOn = false;
	} m_mock;

};

}  // namespace cz
