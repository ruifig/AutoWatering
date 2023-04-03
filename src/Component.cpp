#include "Component.h"

namespace cz
{

namespace
{
	DoublyLinkedList<Component> gComponents;
}

Component::Component()
	: m_ticker(this)
{
	gComponents.pushBack(this);
}

Component::~Component()
{
	gComponents.remove(this);
}

float Component::tickAll(float deltaSeconds)
{
	float countdown = 60*60;
	for(auto&& component : gComponents)
	{
		countdown = std::min(component->m_ticker.tick(deltaSeconds), countdown);
	}

	return countdown;
}

void Component::raiseEvent(const Event& evt)
{
	evt.log();
	for(auto&& component : gComponents)
	{
		component->onEvent(evt);
	}
}

} // namespace cz

