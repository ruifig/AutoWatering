#pragma once

#include "Component.h"

namespace cz
{

class BatteryLife : public Component
{
  public:
	BatteryLife();

  private:
	virtual const char* getName() const override { return "BatteryLife"; }
	virtual bool initImpl() override { return true; }
	virtual float tick(float deltaSeconds) override;
	virtual void onEvent(const Event& evt) override;
	int m_lastPercentageValue = 0;
};

} // namespace cz
