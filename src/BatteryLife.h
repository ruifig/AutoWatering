#pragma once

#include "Config.h"
#include "Component.h"

namespace cz
{

class BatteryLife : public Component
{
  public:
	BatteryLife() {}

  private:
	virtual float tick(float deltaSeconds) override;
	virtual void onEvent(const Event& evt) override;
	int m_lastPercentageValue = 0;
};

} // namespace cz
