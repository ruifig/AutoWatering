#include "BatteryLife.h"
#include "utility/BatteryLifeCalculator.h"
#include <crazygaze/micromuc/Profiler.h>

namespace cz
{

BatteryLife::BatteryLife()
{
	// We only start ticking when we ready a ConfigReady event
	stopTicking();
}

float BatteryLife::tick(float deltaSeconds)
{
	PROFILE_SCOPE(F("BatteryLife::tick"));

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

	return 60.0f;
}

void BatteryLife::onEvent(const Event& evt)
{
	switch (evt.type)
	{
		case Event::ConfigReady:
		{
			startTicking();
		}
		break;
	}
}

#if AW_BATTERYLIFE_ENABLED
BatteryLife gBatteryLife;
#endif

}
