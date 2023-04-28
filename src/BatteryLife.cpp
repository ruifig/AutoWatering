#include "BatteryLife.h"
#include "utility/BatteryLifeCalculator.h"

namespace cz
{

float BatteryLife::tick(float deltaSeconds)
{
	BatteryLifeCalculator batteryLifeCalculator(
		AW_BATTERYLIFE_PIN,
		AW_BATTERYLIFE_R1_VALUE,
		AW_BATTERYLIFE_R2_VALUE,
		AW_BATTERYLIFE_VBAT_0,
		AW_BATTERYLIFE_VBAT_100,
		AW_BATTERYLIFE_VBAT_ADJUSTMENT);

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

#if AW_BATTERYLIFE_ENABLED
BatteryLife gBatteryLife;
#endif

}
