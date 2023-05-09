#include "Component.h"
#include <string.h>
#include <crazygaze/micromuc/Logging.h>
#include <crazygaze/micromuc/Profiler.h>

namespace cz
{

//
// Command
//
bool Command::parseCmd()
{
	parse(src, fullCmd);
	CZ_LOG(logDefault, Log, "Trying to process command: %s", fullCmd);
	cmd = fullCmd;

	// Check if the command is int he form of COMPONENT.COMMAND
	detail::skipToAfter(cmd, '.');
	if (*cmd != 0 && cmd != fullCmd)
	{
		int componentNameLen = cmd - fullCmd - 1;
		CZ_LOG(logDefault, Log, "name length = %d", componentNameLen);
		char componentName[componentNameLen + 1];
		memcpy(componentName, fullCmd, componentNameLen);
		componentName[componentNameLen] = 0;

		targetComponent = Component::getByName(componentName);
		if (!targetComponent)
		{
			CZ_LOG(logDefault, Error, "No component with name '%s' found", componentName);
			return false;
		}
		CZ_LOG(logDefault, Log, "Component '%s', command '%s'", targetComponent ? targetComponent->getName() : "", cmd);
	}
	else
	{
		cmd = fullCmd;
		CZ_LOG(logDefault, Log, "Command '%s'", cmd);
	}

	return true;
}



namespace
{
	DoublyLinkedList<Component> gComponents;
}

//
// Component
//

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

Component* Component::getByName(const char* name)
{
	for(auto&& component : gComponents)
	{
		if (strcasecmp(component->getName(), name) == 0)
		{
			return component;
		}
	}

	return nullptr;
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
	PROFILE_SCOPE(F("Component::tickAll"));

	float countdown = 60*60;
	Component* lowestCountdownCulprit = nullptr;
	for(auto&& component : gComponents)
	{
		float componentCountdown = component->m_ticker.tick(deltaSeconds);
		if (componentCountdown < countdown)
		{
			lowestCountdownCulprit = component;	
			countdown = componentCountdown;
		}
	}

	if (lowestCountdownCulprit)
	{
		CZ_LOG(logDefault, Verbose, "Component causing fastest tick rate: %s. Ticking in %d ms", lowestCountdownCulprit->getName(), static_cast<int>(countdown * 1000));
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

void Component::stopTicking()
{
	m_ticker.stop();
}

void Component::startTicking()
{
	m_ticker.start(1.0f);
}

} // namespace cz

