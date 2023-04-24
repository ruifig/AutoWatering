#pragma once

#include "Component.h"

/*
The RP2040 watchdog can't be stopped once started, which can be a problem when making calls to code that takes too long to complete.
For example, connecting to WiFi can take longer than 8300ms, which causes the Watchdog to trigger.
One way to work around this is to fake a watchdog pause by automatically ticking in a separate FreeRTOS task when we need to cal such code.
It works fine, but at the moment of writing there are a few problems using FreeRTOS with Arduino-Pico, such as problems using WiFi.
See: https://github.com/earlephilhower/arduino-pico/pull/1395

As such, it's recommend this stays set to 0 so FreeRTOS is not used.
*/
#define WATCHDOG_PAUSE_SUPPORT 0

#if WATCHDOG_PAUSE_SUPPORT
	// FreeRTOS forward declarations
	struct tskTaskControlBlock;
	typedef struct tskTaskControlBlock * TaskHandle_t;
#endif

namespace cz
{

class Watchdog : public Component
{
public:
	Watchdog();
	virtual ~Watchdog();
	void heartbeat();

#if WATCHDOG_PAUSE_SUPPORT
	void pause();
	void resume();
#else
	void pause() {}
	void resume() {}
#endif

private:
	virtual const char* getName() const override { return "Watchdog"; }
	virtual bool initImpl() override;
	virtual float tick(float deltaSeconds) override;
	virtual void onEvent(const Event& evt) override;
	virtual bool processCommand(const Command& cmd) override;

	void start();

	bool m_started = false;

#if WATCHDOG_PAUSE_SUPPORT
	int m_pauseCount = 0;
	TaskHandle_t m_autoResetTaskHandle;
#endif
};

#if WATCHDOG_ENABLED
	extern Watchdog gWatchdog;

	struct WatchdogPauseScope
	{
		WatchdogPauseScope(const WatchdogPauseScope&) = delete;
		WatchdogPauseScope& operator=(const WatchdogPauseScope&) = delete;
		WatchdogPauseScope()
		{
			gWatchdog.pause();
		}
		~WatchdogPauseScope()
		{
			gWatchdog.resume();
		}
	};
#else
	struct WatchdogPauseScope
	{
	};
#endif


} // namespace cz
