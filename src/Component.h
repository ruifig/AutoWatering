#pragma  once

#include "crazygaze/micromuc/czmicromuc.h"
#include "crazygaze/micromuc/LinkedList.h"
#include "crazygaze/micromuc/Ticker.h"
#include "Events.h"

namespace cz
{

class Component : public DoublyLinked<Component>
{

public:
	Component();
	virtual ~Component();

	virtual float tick(float deltaSeconds) = 0;
	virtual void onEvent(const Event& evt) = 0;

	static void raiseEvent(const Event& evt);
	static float tickAll(float deltaSeconds);
private:
	TTicker<Component*, float> m_ticker;
};

} // namespace cz

