#pragma once

#include "Component.h"

namespace cz
{

class Watchdog : public Component
{
public:
	Watchdog();
	virtual ~Watchdog();
	void heartbeat();

private:
	virtual const char* getName() const override { return "Watchdog"; }
	virtual bool initImpl() override;
	virtual float tick(float deltaSeconds) override;
	virtual void onEvent(const Event& evt) override;

	void start();

	bool m_started = false;
};

#if WATCHDOG_ENABLED
	extern Watchdog gWatchdog;
#endif

} // namespace cz
