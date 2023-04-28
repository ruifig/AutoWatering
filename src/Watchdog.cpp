#include "Watchdog.h"
#include <Arduino.h>

#if AW_WATCHDOG_PAUSE_SUPPORT
	#include <FreeRTOS.h>
	#include <task.h>

namespace
{
	void autoResetTask(void* pvParameters)
	{
		while(true)
		{
			rp2040.wdt_reset();
			delay(250);
		}
	}
}

#endif

namespace cz
{

Watchdog::Watchdog()
	: Component(ListInsertionPosition::Front)
{
}

Watchdog::~Watchdog()
{
#if AW_WATCHDOG_PAUSE_SUPPORT
	if (m_autoResetTaskHandle)
	{
		vTaskDelete(m_autoResetTaskHandle);
	}
#endif
}

void Watchdog::start()
{
	if (m_started)
	{
		return;
	}

	const int ms = 8300;
	CZ_LOG(logDefault, Log, "Starting rp2040 watchdog timer (%d ms)", ms);
	// According to the documentation (https://arduino-pico.readthedocs.io/en/latest/rp2040.html#hardware-watchdog)
	// the maximum wait is 8.3 seconds. 
	rp2040.wdt_begin(ms);
	m_started = true;
}

bool Watchdog::initImpl()
{
#if AW_WATCHDOG_PAUSE_SUPPORT
	start();
	CZ_LOG(logDefault, Log, "Creating watchdog auto reset task (to support pause/resume)");
	xTaskCreate(
		autoResetTask, // Function
		"WatchdogAutoResetTask", // Task name
		configMINIMAL_STACK_SIZE,
		nullptr, // passed to the task
		1,
		&m_autoResetTaskHandle
	);

	vTaskSuspend(m_autoResetTaskHandle);
#else
	if constexpr(AW_WIFI_ENABLED)
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
#endif
	
	return true;
}

#if AW_WATCHDOG_PAUSE_SUPPORT
void Watchdog::pause()
{
	if (m_pauseCount==0)
	{
		CZ_LOG(logDefault, Log, "Pausing watchdog");
		vTaskResume(m_autoResetTaskHandle);
	}

	++m_pauseCount;
}

void Watchdog::resume()
{
	--m_pauseCount;
	if (m_pauseCount==0)
	{
		CZ_LOG(logDefault, Log, "Resuming watchdog");
		vTaskSuspend(m_autoResetTaskHandle);
	}
}
#endif


void Watchdog::heartbeat()
{
	//CZ_LOG(logDefault, Log, "*********** Watchdog heartbeat **********")
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
		case Event::Type::WifiStatus:
		{
			auto&& e = static_cast<const WifiStatusEvent&>(evt);
			start();
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
	else if (cmd.is("reboot"))
	{
		rp2040.reboot();
		return true;
	}
#if AW_WATCHDOG_PAUSE_SUPPORT
	else if (cmd.is("pause"))
	{
		pause();
		return true;
	}
	else if (cmd.is("resume"))
	{
		resume();
		return true;
	}
#endif

	return false;
}

#if AW_WATCHDOG_ENABLED
	Watchdog gWatchdog;
#endif

} // namespace cz
