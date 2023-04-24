#pragma once

#include "Component.h"
#include <crazygaze/micromuc/SerialStringReader.h>

namespace cz
{

class CommandConsole : public Component
{
public:
	CommandConsole();

private:

	//
	// Component interface
	//
	virtual const char* getName() const override { return "CommandConsole"; }
	virtual bool initImpl() override;
	virtual float tick(float deltaSeconds) override;
	virtual void onEvent(const Event& evt) override { }
	virtual bool processCommand(const Command& cmd);

	SerialStringReader<> m_serialStringReader;
};

#if COMMAND_CONSOLE_ENABLED
	extern CommandConsole gCommandConsole;

	#if !(CZ_LOG_ENABLED && CZ_SERIAL_LOG_ENABLED)
		#error CZ_LOG_ENABLED and CZ_SERIAL_LOG_ENABLED must be set to 1 to be able to set COMMAND_CONSOLE_ENABLED
	#endif
#endif

} // namespace cz
