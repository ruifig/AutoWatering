#pragma once

#include "Component.h"

#if AW_WATCHDOG_PAUSE_SUPPORT
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

#if AW_WATCHDOG_PAUSE_SUPPORT
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

#if AW_WATCHDOG_PAUSE_SUPPORT
	int m_pauseCount = 0;
	TaskHandle_t m_autoResetTaskHandle;
#endif
};

#if AW_WATCHDOG_ENABLED
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
