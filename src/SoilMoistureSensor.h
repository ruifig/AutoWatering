#pragma once

#include <Arduino.h>
#include <assert.h>
#include "Component.h"
#include "SemaphoreQueue.h"

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
class RealSoilMoistureSensor : public Component
{
  public:

	/**
	 * @param index Sensor's index within the program data
	 * @param vinPin Pin used to control power to the sensor. If power is always on, you can use a DummyDigitalPin instance
	 * @param dataPin What multiplexer pin we are using to read the sensor
	 */
	RealSoilMoistureSensor(uint8_t index, DigitalOutputPin& vinPin, AnalogInputPin& dataPin);

	// Disable copying
	RealSoilMoistureSensor(const RealSoilMoistureSensor&) = delete;
	const RealSoilMoistureSensor& operator=(const RealSoilMoistureSensor&) = delete;


	//
	// Component interface
	//
  public:
	virtual const char* getName() const override;
  protected:
	virtual bool initImpl() override;
	virtual float tick(float deltaSeconds) override;
	virtual void onEvent(const Event& evt) override;

	enum class State : uint8_t
	{
		Initializing,
		PoweredDown,
		QueuedForReading, // Sensor needs a reading, and it's waiting its turn, to respect AW_MAX_SIMULTANEOUS_MOISTURESENSORS
		Reading // Sensor is powering up to perform a read
	};

	static const char* const ms_stateNames[3];

	String m_name;

	// Seconds since last sensor read.
	// Keeping this separate from m_timeInState, so pausing the group or entering a menu doesn't cause the sensor reading timer
	// to reset
	// Setting this initially to FLT_MAX, so we do a reading for all sensors once we boot up.
	float m_timeSinceLastRead = __FLT_MAX__;

	float m_timeInState = 0;
	float m_nextTickWait = 0;
	State m_state = State::Initializing;
	uint8_t m_index;

	// Pin used to control power
	DigitalOutputPin& m_vinPin;
	AnalogInputPin& m_dataPin;

	using SemaphoreQueue = TSemaphoreQueue<uint8_t, AW_MAX_NUM_PAIRS, AW_MAX_SIMULTANEOUS_MOISTURESENSORS>;
	static SemaphoreQueue ms_semaphoreQueue;
	SemaphoreQueue::Handle m_queueHandle;

	virtual SensorReading readSensor();

	void changeToState(State newState);
	void onLeaveState();
	void onEnterState();
	void tryEnterReadingState();
};

class MockSoilMoistureSensor : public RealSoilMoistureSensor
{
public:
	using RealSoilMoistureSensor::RealSoilMoistureSensor;

	//
	// Component interface
	//
	virtual bool initImpl() override;
	virtual float tick(float deltaSeconds) override;
	virtual void onEvent(const Event& evt) override;

protected:

	virtual SensorReading readSensor() override;

	static constexpr int ms_delayChaseValue = 5;

	struct
	{
		int dryValue;
		int waterValue;

		// To simulate error conditions
		SensorReading::Status status = SensorReading::Status::Valid;

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

#if AW_MOCK_COMPONENTS
	class SoilMoistureSensor : public MockSoilMoistureSensor
	{
		using MockSoilMoistureSensor::MockSoilMoistureSensor;
	};
#else
	class SoilMoistureSensor : public RealSoilMoistureSensor
	{
		using RealSoilMoistureSensor::RealSoilMoistureSensor;
	};
#endif

}  // namespace cz
