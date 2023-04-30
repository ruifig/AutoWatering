#pragma once

#include "Component.h"

namespace cz
{

class MQTTUI : public Component
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
};

} // namespace cz
