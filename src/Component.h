#pragma  once

#include "crazygaze/micromuc/czmicromuc.h"
#include "Events.h"

namespace cz
{

class Component
{

public:
	Component();
	virtual ~Component();

	virtual float tick(float deltaSeconds) = 0;
	virtual void onEvent(const Event& evt) = 0;
	static void raiseEvent(const Event& evt);

private:
	static Component* ms_first;
	static Component* ms_last;
	Component* m_next;
	Component* m_previous;
};

} // namespace cz

