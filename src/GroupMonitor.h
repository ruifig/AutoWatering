#pragma once

#include "Config/Config.h"
#include "Component.h"
#include "SemaphoreQueue.h"

namespace cz
{

class GroupMonitor : public Component
{
  public:
	GroupMonitor(uint8_t index, IOExpanderPinInstance motorPin);

	// Initiates an explicit shot
	// If the motor is already running, it does nothing
	void doShot();

	// This should not be use externally except for testing
	void turnMotorOff();
	
  protected:

	//
	// Component interface
	//
	virtual const char* getName() const override;
	virtual bool initImpl() override;
	virtual float tick(float deltaSeconds) override;
	virtual void onEvent(const Event& evt) override;
	virtual bool processCommand(const Command& cmd) override;

	/**
	 * Tries to turn the motor on
	 * \param registerInterest If true it will register interest in the semaphore queue, so it goes up in priority as multiple tries are done
	 * \return true if motor was started
	 */
	bool tryTurnMotorOn(bool registerInterest);

	uint8_t m_index;
	IOExpanderPinInstance m_motorPin;

	// Tells if there was a valid sensor reading since the last watering.
	// This is used to make sure we don't turn on the motor unless there was a recent sensor reading
	bool m_sensorValidReadingSinceLastShot;
	SensorReading m_lastValidReading;

	// This serves two purposes
	// * If >0 then it means the motor is ON and we're counting down to turn it off
	// * If <= 0 then it means the motor is OFF, and...
	//		* If <= (-MINIMUM_TIME_BETWEEN_MOTOR_ON) then we can do another sensor sheck
	float m_motorOffCountdown = -MINIMUM_TIME_BETWEEN_MOTOR_ON;

	using SemaphoreQueue = TSemaphoreQueue<uint8_t, MAX_NUM_PAIRS, MAX_SIMULTANEOUS_MOTORS>;
	static SemaphoreQueue ms_semaphoreQueue;
	SemaphoreQueue::Handle m_queueHandle;
};

} // namespace cz
