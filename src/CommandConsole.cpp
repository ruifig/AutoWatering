#include "CommandConsole.h"
#include <crazygaze/micromuc/Profiler.h>

namespace cz
{

//
// CommandConsole
//
CommandConsole::CommandConsole()
{
}

bool CommandConsole::initImpl()
{
	m_serialStringReader.begin(AW_CUSTOM_SERIAL);
	return true;
}

float CommandConsole::tick(float deltaSeconds)
{
	PROFILE_SCOPE(F("CommandConsole::tick"));

	//CZ_LOG(logDefault, Log, "CommandConsole::tick");
	while (m_serialStringReader.tryRead())
	{
		//CZ_LOG(logDefault, Log, "    %u", millis());

		Command cmd(m_serialStringReader.retrieve());
		if (!cmd.parseCmd())
		{
			continue;
		}

		if (cmd.targetComponent == nullptr)
		{
			cmd.targetComponent = this;
		}

		if (!cmd.targetComponent->processCommand(cmd))
		{
			CZ_LOG(logDefault, Error, "Failed to execute %s.%s command", cmd.targetComponent->getName(), cmd.cmd);
		}

	}
	return 0.250f;
}


volatile int gProfilerCount = 0;
bool CommandConsole::processCommand(const Command& cmd)
{
	if (cmd.is("setdevicename"))
	{
		char deviceName[AW_DEVICENAME_MAX_LEN+1];
		if (cmd.parseParams(deviceName))
		{
			gCtx.data.setDeviceName(deviceName);
			return true;
		}
	}
	else if (cmd.is("profiler_log"))
	{
		gProfilerCount++; 
		PROFILER_LOG();
		gProfilerCount--; 
		return true;
	}
	else if (cmd.is("profiler_reset"))
	{
		PROFILER_RESET();
		return true;
	}
	else if (cmd.is("heapinfo"))
	{
		CZ_LOG(logDefault, Log, "HEAP INFO: size=%d, used=%d, free=%d", rp2040.getTotalHeap(), rp2040.getUsedHeap(),
		       rp2040.getFreeHeap());
		return true;
	}
	else if (cmd.is("setgroupthreshold"))
	{
		int idx, value;
		if (cmd.parseParams(idx, value) && idx < AW_MAX_NUM_PAIRS)
		{
			gCtx.data.getGroupData(idx).setThresholdValue(value);
			return true;
		}
	}
	else if (cmd.is("setgroupthresholdaspercentage"))
	{
		int idx, value;
		if (cmd.parseParams(idx, value) && idx < AW_MAX_NUM_PAIRS)
		{
			gCtx.data.getGroupData(idx).setThresholdValueAsPercentage(value);
			return true;
		}
	}
	else if (cmd.is("startgroup"))
	{
		int idx;
		if (cmd.parseParams(idx) && idx < AW_MAX_NUM_PAIRS)
		{
			gCtx.data.getGroupData(idx).setRunning(true);
			return true;
		}
	}
	else if (cmd.is("stopgroup"))
	{
		int idx;
		if (cmd.parseParams(idx) && idx < AW_MAX_NUM_PAIRS)
		{
			gCtx.data.getGroupData(idx).setRunning(false);
			return true;
		}
	}
	else if (cmd.is("logconfig"))
	{
		gCtx.data.logConfig();
		return true;
	}
	else if (cmd.is("loggroupconfig"))
	{
		int idx;
		if (cmd.parseParams(idx) && idx < AW_MAX_NUM_PAIRS)
		{
			gCtx.data.getGroupData(idx).logConfig();
			return true;
		}
	}
	else if (cmd.is("selectgroup"))
	{
		int8_t idx;
		if (cmd.parseParams(idx) && idx < AW_MAX_NUM_PAIRS)
		{
			gCtx.data.trySetSelectedGroup(idx);
			return true;
		}
	}
	else if (cmd.is("setmocksensorerrorstatus"))
	{
		int idx, status;
		if (cmd.parseParams(idx, status))
		{
			if (status >= SensorReading::Status::First && status <= SensorReading::Status::Last)
			{
				Component::raiseEvent(SetMockSensorErrorStatusEvent(idx, static_cast<SensorReading::Status>(status)));
				return true;
			}
			else
			{
				CZ_LOG(logDefault, Error, F("Invalid status value"));
			}
		}
	}
	else if (cmd.is("setmocksensor"))
	{
		int idx, value;
		if (cmd.parseParams(idx, value) && idx < AW_MAX_NUM_PAIRS)
		{
			Component::raiseEvent(SetMockSensorValueEvent(idx, value));
			return true;
		}
	}
	else if (cmd.is("setmocksensors"))
	{
		int value;
		if (cmd.parseParams(value))
		{
			for (int idx = 0; idx < AW_MAX_NUM_PAIRS; idx++)
			{
				Component::raiseEvent(SetMockSensorValueEvent(idx, value));
			}
			return true;
		}
	}
	else if (cmd.is("save"))
	{
		gCtx.data.save();

		ProgramData prgData(gCtx);
		prgData.begin();
		prgData.load();
		prgData.logConfig();
		return true;
	}
	else if (cmd.is("savegroup"))
	{
		uint8_t idx;
		if (cmd.parseParams(idx) && idx < AW_MAX_NUM_PAIRS)
		{
			gCtx.data.saveGroupConfig(idx);
			return true;
		}
	}
	else if (cmd.is("load"))
	{
		gCtx.data.load();
		return true;
	}
	else if (cmd.is("setverbosity"))
	{
		char name[30];
		int verbosity;
		constexpr int minVerbosity = static_cast<int>(LogVerbosity::Fatal);
		constexpr int maxVerbosity = static_cast<int>(LogVerbosity::Verbose);
		if (cmd.parseParams(name, verbosity))
		{
			if (LogCategoryBase* category = LogCategoryBase::find(name))
			{
				if (verbosity >= minVerbosity && verbosity <= maxVerbosity)
				{
					LogVerbosity v = static_cast<LogVerbosity>(verbosity);
					category->setVerbosity(v);
					CZ_LOG(logDefault, Log, F("\"%s\" verbosity set to %s"), name, logVerbosityToString(v));
					return true;
				}
				else
				{
					CZ_LOG(logDefault, Error, F("Log verbosity needs to be from %d to %d"), minVerbosity, maxVerbosity);
				}
			}
			else
			{
				CZ_LOG(logDefault, Error, F("Log category \"%s\" doesn't exist"), name);
			}
		}
	}


	CZ_LOG(logDefault, Error, F("Command \"%s\" not recognized"), m_serialStringReader.retrieve());
	return false;
}

#if AW_COMMAND_CONSOLE_ENABLED
	CommandConsole gCommandConsole;
#endif

} // namespace cz
