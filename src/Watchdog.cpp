#include "Watchdog.h"
#include <Arduino.h>

namespace cz
{

Watchdog::Watchdog()
{
}

Watchdog::~Watchdog()
{
}

float Watchdog::tick(float deltaSeconds)
{
	if (m_started)
	{
		rp2040.wdt_reset();
	}
	else
	{
		const int ms = 8300;
		CZ_LOG(logDefault, Log, "Starting rp2040 watchdog timer (%d ms)", ms);
		// According to the documentation (https://arduino-pico.readthedocs.io/en/latest/rp2040.html#hardware-watchdog)
		// the maximum wait is 8.3 seconds. 
		rp2040.wdt_begin(ms);
		m_started = true;
	}

	return 0.250f;
}

void Watchdog::onEvent(const Event& evt)
{
}

#if WATCHDOG_ENABLED
	Watchdog gWatchdog;
#endif

} // namespace cz
