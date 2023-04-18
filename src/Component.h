#pragma  once

#include "crazygaze/micromuc/czmicromuc.h"
#include "crazygaze/micromuc/LinkedList.h"
#include "crazygaze/micromuc/Ticker.h"
#include "Events.h"
#include <crazygaze/micromuc/StringUtils.h>

namespace cz
{

struct Command;

class Component : public DoublyLinked<Component>
{
public:

	/*
	* Where to insert the component
	*/
	enum class ListInsertionPosition
	{
		Front, // Insertion at the beginning of the list
		Back // Insertion at the end of the list
	};

	explicit Component(ListInsertionPosition insertPos = ListInsertionPosition::Back);
	virtual ~Component();

	bool isInitialized() const
	{
		return m_initialized;
	}

	/*
	* \return
	*	Returns true if finished, false if it should be called again
	*
	* This allows our setup() to make multiple passes to initialize all components, where each component might have dependencies which are not initialized yet,
	* so it delays it's own initialization by returning false until all dependencies are initialized.
	*
	* Derived classed should implement initImpl()
	*/
	bool init();

	virtual const char* getName() const = 0;
	virtual float tick(float deltaSeconds) = 0;
	virtual void onEvent(const Event& evt) = 0;
	virtual bool processCommand(const Command& cmd) { return true; }

	static Component* getByName(const char* name);
	static void raiseEvent(const Event& evt);
	static void initAll();
	static float tickAll(float deltaSeconds);
	static int getCount();
private:
	virtual bool initImpl() = 0;

	TTicker<Component*, float> m_ticker;
	bool m_initialized = false;
};

struct Command
{
	// Pointer to the full command string (command + parameters)
	const char* src = nullptr;

	// Full command name (e.g: COMPONENT.COMMAND )
	char fullCmd[60];
	const char* cmd; // Pointer into fullCmd, where the actual command starts
	Component* targetComponent = nullptr;

	explicit Command(const char* src)
		: src(src)
	{
	}

	bool parseCmd();

	bool is(const char* cmd) const
	{
		return strcasecmp_P(this->cmd, cmd) == 0 ? true : false;
	}

	template<typename... Params>
	bool parseParams(Params&... params) const
	{
		const char* tmp = src;
		if (parse(tmp, params...))
		{
			return true;
		}
		else
		{
			CZ_LOG(logDefault, Error, F("Error parsing parameters for command \"%s\""), cmd);
			return false;
		}
	}

};

} // namespace cz

