#pragma once

#include "Component.h"

namespace cz
{

class Watchdog : public Component
{
public:
	Watchdog();
	virtual ~Watchdog();

	virtual float tick(float deltaSeconds) override;
	virtual void onEvent(const Event& evt) override;
private:
	bool m_started = false;
};

#if WATCHDOG_ENABLED
	extern Watchdog gWatchdog;
#endif

} // namespace cz
