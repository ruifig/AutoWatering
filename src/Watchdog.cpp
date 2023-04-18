#include "Watchdog.h"
#include <Arduino.h>

namespace cz
{

Watchdog::Watchdog()
	: Component(ListInsertionPosition::Front)
{
}

Watchdog::~Watchdog()
{
}

void Watchdog::start()
{
	const int ms = 8300;
	CZ_LOG(logDefault, Log, "Starting rp2040 watchdog timer (%d ms)", ms);
	// According to the documentation (https://arduino-pico.readthedocs.io/en/latest/rp2040.html#hardware-watchdog)
	// the maximum wait is 8.3 seconds. 
	rp2040.wdt_begin(ms);
	m_started = true;
}

bool Watchdog::initImpl()
{
	if constexpr(WIFI_ENABLED)
	{
		// If we are using WIFI, we need to wait until we either connect or fail to connect.
		// This is because connecting to Wifi can take over the 8.3 secs limit of the watchdog. :(
		// So once we detect that Wifi connection was attempted, we initialized the watchdog
		CZ_LOG(logDefault, Log, "Delaying watchdog start until wifi conneciton is attempted.");
	}
	else
	{
		start();
	}
	
	return true;
}

void Watchdog::heartbeat()
{
	rp2040.wdt_reset();
}

float Watchdog::tick(float deltaSeconds)
{
	heartbeat();
	return 0.250f;
}

void Watchdog::onEvent(const Event& evt)
{
	switch(evt.type)
	{
		case Event::Type::Wifi:
		{
			auto&& e = static_cast<const WifiEvent&>(evt);
			if (!m_started)
			{
				start();
			}
		}
		break;
	}
}

bool Watchdog::processCommand(const Command& cmd)
{
	// Blocks the program for X ms, so simulate a freeze
	if (cmd.is("freeze"))
	{
		int ms;
		if (cmd.parseParams(ms))
		{
			delay(ms);
			return true;
		}
	}

	return false;
}

#if WATCHDOG_ENABLED
	Watchdog gWatchdog;
#endif

} // namespace cz
