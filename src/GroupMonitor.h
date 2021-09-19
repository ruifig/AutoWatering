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
	
  protected:
	void turnMotorOn(bool direction);
	void turnMotorOff();

	Context& m_ctx;
	IOExpanderPin m_motorPin1;
	IOExpanderPin m_motorPin2;
	uint8_t m_index;

	float m_totalTime = 0;
	bool isOn = false;

};

} // namespace cz