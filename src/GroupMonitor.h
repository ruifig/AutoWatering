#pragma once

#include "Config.h"
#include "Context.h"
#include "Component.h"

namespace cz
{

class GroupMonitor : public Component
{
  public:
	GroupMonitor(Context& ctx, uint8_t index, IOExpanderPin motorPin1, IOExpanderPin motorPin2);

	void begin();
	virtual float tick(float deltaSeconds) override;
	virtual void onEvent(const Event& evt) override;

	// This should not be use externally except for testing
	void turnMotorOn(bool direction = true);
	// This should not be use externally except for testing
	void turnMotorOff();
	
  protected:

	Context& m_ctx;
	IOExpanderPin m_motorPin1;
	IOExpanderPin m_motorPin2;
	uint8_t m_index;

	// This serves two purposes
	// * If >0 then it means the motor is ON and we're counting down to turn it off
	// * If <= 0 then it means the motor is OFF, and...
	//		* If <= (-MINIMUM_TIME_BETWEEN_MOTOR_ON) then we can do another sensor sheck
	float m_motorOffCountdown = -MINIMUM_TIME_BETWEEN_MOTOR_ON;
};

} // namespace cz