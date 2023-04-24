#include "BatteryLife.h"
#include "utility/BatteryLifeCalculator.h"

namespace cz
{

float BatteryLife::tick(float deltaSeconds)
{
	BatteryLifeCalculator batteryLifeCalculator(BATTERY_LIFE_PIN, 267200, 459600, 2.5f, 4.2f, 1.07f);
	float voltage = 0;
	int newPerc = batteryLifeCalculator.readBatteryLife(&voltage);
	if (newPerc != m_lastPercentageValue)
	{
		m_lastPercentageValue = newPerc;
		Component::raiseEvent(BatteryLifeReadingEvent(newPerc, voltage));
	}

	return 10.0f;
}

void BatteryLife::onEvent(const Event& evt)
{
}

#if BATTERY_LIFE_ENABLED
BatteryLife gBatteryLife;
#endif

}
