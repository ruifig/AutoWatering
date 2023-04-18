#include "Component.h"

namespace cz
{

namespace
{
	DoublyLinkedList<Component> gComponents;
}

Component::Component(Component::ListInsertionPosition insertPos)
	: m_ticker(this)
{
	if (insertPos == Component::ListInsertionPosition::Front)
	{
		gComponents.pushFront(this);
	}
	else
	{
		gComponents.pushBack(this);
	}
}

Component::~Component()
{
	gComponents.remove(this);
}

bool Component::init()
{
	if (m_initialized)
	{
		return true;
	}
	else
	{
		CZ_LOG(logDefault, Log, "Initializing component '%s' ...", getName());
		m_initialized = initImpl();
		CZ_LOG(logDefault, Log, "        %s", m_initialized ? "DONE" : "DELAYED");
		return m_initialized;
	}
}

void Component::initAll()
{
	int loopCount = 0;

	//
	// Loop through all components until all are initialized or we detect we've tried too often
	//
	while(true)
	{
		loopCount++;
		bool repeat = false;
		int componentCount;
		for(auto&& component : gComponents)
		{
			componentCount++;
			if(!component->init())
			{
				repeat = true;
			}
		}

		if (repeat)
		{
			if (loopCount >= componentCount)
			{
				CZ_LOG(logDefault, Error, "Failed to initialize all components. Restarting in 10 seconds");
				delay(10000);
				rp2040.reboot();
			}
		}
		else
		{
			return;
		}
	}
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

