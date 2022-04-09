#pragma once

#include "Config.h"
#include "Component.h"

namespace cz
{

class GroupMonitor : public Component
{
  public:
	GroupMonitor(uint8_t index, IOExpanderPin motorPin);

	void begin();
	virtual float tick(float deltaSeconds) override;
	virtual void onEvent(const Event& evt) override;

	// Initiates an explicit shot
	// If the motor is already running, it does nothing
	void doShot();

	// This should not be use externally except for testing
	void turnMotorOff();
	
  protected:

	void turnMotorOn();

	uint8_t m_index;
	IOExpanderPin m_motorPin;

	// Tells if there was a valid sensor reading since the last watering.
	// This is used to make sure we don't turn on the motor unless there was a recent sensor reading
	bool m_sensorValidReadingSinceLastShot;
	SensorReading m_lastValidReading;

	// This serves two purposes
	// * If >0 then it means the motor is ON and we're counting down to turn it off
	// * If <= 0 then it means the motor is OFF, and...
	//		* If <= (-MINIMUM_TIME_BETWEEN_MOTOR_ON) then we can do another sensor sheck
	float m_motorOffCountdown = -MINIMUM_TIME_BETWEEN_MOTOR_ON;
};

} // namespace cz
