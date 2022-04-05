#include "Config.h"

#if __MBED__
	#include "utility/PluggableUSBDevice_fix.h"
#endif

#if 0
#include "Adafruit_MCP23017.h"
#include "utility/MCP23017Wrapper.h"
#include "crazygaze/micromuc/SerialStringReader.h"
#include "crazygaze/micromuc/Logging.h"
#include "crazygaze/micromuc/StringUtils.h"
#include "utility/MuxNChannels.h"
#include "gfx/MyDisplay1.h"
#include "gfx/MyXPT2046.h"
#endif

#if 1 || PORTING_TO_RP2040

#include "Config.h"
#include "Context.h"
#include "SoilMoistureSensor.h"
#include "GroupMonitor.h"
#include "DisplayTFT.h"
#include "crazygaze/micromuc/Ticker.h"
#include "crazygaze/micromuc/Logging.h"
#include <algorithm>
#include <utility>
#include <memory>

#include "crazygaze/micromuc/SDLogOutput.h"
#include "crazygaze/micromuc/Profiler.h"
#include "crazygaze/micromuc/SerialStringReader.h"

using namespace cz;

#if SD_CARD_LOGGING
	SDCardHelper gSDCard;
	SDLogOutput gSdLogOutput;
#endif

#if 1
	// Use normal ticking, where the object is only ticked as often as it wants
	using TickingMethod = TickerPolicy::TTime<float>;
#else
	// Use forced ticking, where the object is always ticked every loop independently of how often it wants to be ticked
	using TickingMethod = TickerPolicy::TTimeAlwaysTick<float>;
#endif

#if MOCK_COMPONENTS
	using SoilMoistureSensorTicker = TTicker<MockSoilMoistureSensor, float, TickingMethod>;
#else
	using SoilMoistureSensorTicker = TTicker<SoilMoistureSensor, float, TickingMethod>;
#endif

SoilMoistureSensorTicker gSoilMoistureSensors[NUM_PAIRS] =
{
	 {true, 0, IO_EXPANDER_VPIN_SENSOR0, MUX_MOISTURE_SENSOR0}
#if NUM_PAIRS>1
	,{true, 1, IO_EXPANDER_VPIN_SENSOR1, MUX_MOISTURE_SENSOR1}
#endif
#if NUM_PAIRS>2
	,{true, 2, IO_EXPANDER_VPIN_SENSOR2, MUX_MOISTURE_SENSOR2}
#endif
#if NUM_PAIRS>3
	,{true, 3, IO_EXPANDER_VPIN_SENSOR3, MUX_MOISTURE_SENSOR3}
#endif
};

using GroupMonitorTicker = TTicker<GroupMonitor, float, TickingMethod>;

GroupMonitorTicker gGroupMonitors[NUM_PAIRS] =
{
	 { true, 0, IO_EXPANDER_MOTOR0}
#if NUM_PAIRS>1
	,{ true, 1, IO_EXPANDER_MOTOR1}
#endif
#if NUM_PAIRS>2
	,{ true, 2, IO_EXPANDER_MOTOR2}
#endif
#if NUM_PAIRS>3
	,{ true, 3, IO_EXPANDER_MOTOR3}
#endif
};


namespace cz
{
	MyDisplay1 gScreen(TFT_PIN_CS, TFT_PIN_DC, TFT_PIN_BACKLIGHT);
	const TouchCalibrationData gTsCalibrationData = {211, 3412, 334, 3449, 4};
	MyXPT2046 gTs(gScreen, TOUCH_PIN_CS, TOUCH_PIN_IRQ, &gTsCalibrationData);
}

namespace
{
	cz::SerialLogOutput gSerialLogOutput;
	cz::SerialStringReader<> gSerialStringReader;
}

TTicker<DisplayTFT, float, TickingMethod> gDisplay(true);

void doGroupShot(uint8_t index)
{
	gGroupMonitors[index].getObj().doShot();
}

unsigned long gPreviousMicros = 0;

void setup()
{
	gSerialLogOutput.begin(Serial1, 115200);
	gSerialStringReader.begin(Serial1);

	//
	// Initialize I2C
	// NOTE: The original Adafruit_MCP23017.cpp code automatically initialized IC2, but that causes some problems
	// because I also need to set the clock before I2C is used, and also because eeprom is also using I2C.
	// So, Adafruit_MCP23017 has a change to disable I2C initialization, and initialization is done here.
	Wire.begin();
	// MCP23017 : 400kHz at 3.3v
	// AT24C256 : 400kHz at 2.7v, 2.5v
	Wire.setClock(400000);


#if SD_CARD_LOGGING
	if (gSDCard.betin(SD_CARD_SS_PIN))
	{
		gSdLogOutput.begin(gSDCard.root, "log.txt", true);
		CZ_LOG(logDefault, Log, "SD card log file initialized");
	}
#endif

	gScreen.begin();
	gTs.begin();
	//gTs.calibrate();

	gCtx.begin();
	gDisplay.getObj().begin();

	for(auto&& ticker : gSoilMoistureSensors)
	{
		ticker.getObj().begin();
	}

	for(auto&& ticker : gGroupMonitors)
	{
		ticker.getObj().begin();
	}

	gPreviousMicros = micros();
}

PROFILER_CREATE(30);

void loop()
{
	PROFILER_STARTRUN();

	unsigned long nowMicros = micros();
	// NOTE: If using subtraction, there is no need to handle wrap around
	// See: https://arduino.stackexchange.com/questions/33572/arduino-countdown-without-using-delay/33577#33577
	float deltaSeconds = (nowMicros - gPreviousMicros) / 1000000.0f;
	float countdown = 60*60;
	
	{
		PROFILE_SCOPE(F("TickAll"));
		countdown = std::min(gDisplay.tick(deltaSeconds), countdown);

		for (auto&& ticker : gSoilMoistureSensors)
		{
			countdown = std::min(ticker.tick(deltaSeconds), countdown);
		}

		for (auto&& ticker : gGroupMonitors)
		{
			countdown = std::min(ticker.tick(deltaSeconds), countdown);
		}
	}
	
	// We use this so that we can just put a breakpoint in here and force the code to run when we want to check the profiler data
	#if CONSOLE_COMMANDS
	{
		if (gSerialStringReader.tryRead())
		{
			char cmd[30];
			const char* src = gSerialStringReader.retrieve(); 
			parse(src, cmd);

			auto parseCommand = [&cmd, &src](auto&... params) -> bool
			{
				if (parse(src, params...))
				{
					return true;
				}
				else
				{
					CZ_LOG(logDefault, Error, F("Error parsing parameters for command \"%s\""), cmd);
					return false;
				}
			};

			if (strcmp_P(cmd, (const char*)F("profiler_log"))==0)
			{
				PROFILER_LOG();
			}
			else if (strcmp_P(cmd, (const char*)F("profiler_reset"))==0)
			{
				PROFILER_RESET();
			}
			else if (strcmp_P(cmd, (const char*)F("motoroff"))==0)
			{
				int idx;
				if (parseCommand(idx) && idx < NUM_PAIRS)
				{
					gGroupMonitors[idx].getObj().turnMotorOff();
				}
			}
			else if (strcmp_P(cmd, (const char*)F("motoron"))==0)
			{
				int idx;
				if (parseCommand(idx) && idx < NUM_PAIRS)
				{
					gGroupMonitors[idx].getObj().doShot();
				}
			}
			else if (strcmp_P(cmd, (const char*)F("setgroupthreshold"))==0)
			{
				int idx, value;
				if (parseCommand(idx, value) && idx < NUM_PAIRS)
				{
					gCtx.data.getGroupData(idx).setThresholdValue(value);
				}
			}
			else if (strcmp_P(cmd, (const char*)F("setgroupthresholdaspercentage"))==0)
			{
				int idx, value;
				if (parseCommand(idx, value) && idx < NUM_PAIRS)
				{
					gCtx.data.getGroupData(idx).setThresholdValueAsPercentage(value);
				}
			}
			else if (strcmp_P(cmd, (const char*)F("startgroup"))==0)
			{
				int idx;
				if (parseCommand(idx) && idx < NUM_PAIRS)
				{
					gCtx.data.getGroupData(idx).setRunning(true);
				}
			}
			else if (strcmp_P(cmd, (const char*)F("stopgroup"))==0)
			{
				int idx;
				if (parseCommand(idx) && idx < NUM_PAIRS)
				{
					gCtx.data.getGroupData(idx).setRunning(false);
				}
			}
			else if (strcmp_P(cmd, (const char*)F("selectgroup"))==0)
			{
				int8_t idx;
				if (parseCommand(idx) && idx < NUM_PAIRS)
				{
					gCtx.data.trySetSelectedGroup(idx);
				}
			}
			else if (strcmp_P(cmd, (const char*)F("setmocksensor"))==0)
			{
				int idx, value;
				if (parseCommand(idx, value) && idx < NUM_PAIRS)
				{
					Component::raiseEvent(SetMockSensorValueEvent(idx, value));
				}
			}
			else if (strcmp_P(cmd, (const char*)F("setmocksensors"))==0)
			{
				int value;
				if (parseCommand(value))
				{
					for(int idx=0; idx<NUM_PAIRS; idx++)
					{
						Component::raiseEvent(SetMockSensorValueEvent(idx, value));
					}
				}
			}
			else if (strcmp_P(cmd, (const char*)F("save"))==0)
			{
				gCtx.data.save();
			}
			else if (strcmp_P(cmd, (const char*)F("savegroup"))==0)
			{
				uint8_t idx;
				if (parseCommand(idx) && idx < NUM_PAIRS)
				{
					gCtx.data.saveGroupConfig(idx);
				}
			}
			else if (strcmp_P(cmd, (const char*)F("load"))==0)
			{
				gCtx.data.load();
			}
			else if (strcmp_P(cmd, (const char*)F("setverbosity"))==0)
			{
				char name[30];
				int verbosity;
				constexpr int minVerbosity = static_cast<int>(LogVerbosity::Fatal);
				constexpr int maxVerbosity = static_cast<int>(LogVerbosity::Verbose);
				if (parseCommand(name, verbosity))
				{
					if(LogCategoryBase* category = LogCategoryBase::find(name))
					{
						if (verbosity>=minVerbosity && verbosity<=maxVerbosity)
						{
							LogVerbosity v = static_cast<LogVerbosity>(verbosity);
							category->setVerbosity(v);
							CZ_LOG(logDefault, Log, F("\"%s\" verbosity set to %s"), name, logVerbosityToString(v));
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
			else
			{
				CZ_LOG(logDefault, Error, F("Command \"%s\" not recognized"), gSerialStringReader.retrieve());
			}
		}
	}
	#endif // CONSOLE_COMMANDS

	gPreviousMicros = nowMicros;
}

#endif

#if 0

namespace
{
	cz::SerialLogOutput gSerialLogOutput;
	cz::SerialStringReader<> gSerialStringReader;
}

namespace cz
{
	MyDisplay1 gScreen(TFT_PIN_CS, TFT_PIN_DC, TFT_PIN_BACKLIGHT);
	const TouchCalibrationData gTsCalibrationData = {211, 3412, 334, 3449, 4};
	MyXPT2046 gTs(gScreen, TOUCH_PIN_CS, TOUCH_PIN_IRQ, &gTsCalibrationData);
}

using namespace cz;

void setup()
{
	gSerialLogOutput.begin(Serial1, 115200);
	gSerialStringReader.begin(Serial1);

	Serial.print("Hello World-1!");
	Serial1.print("Hello World-2!");
	CZ_LOG(logDefault, Log, "Hello World-3!");
	CZ_LOG(logDefault, Log, F("Hello World-4!"));

	gScreen.begin();
	gTs.begin();
	//gTs.calibrate();
}

void tryReadString()
{
	if (!gSerialStringReader.tryRead())
	{
		return;
	}

	const char* src = gSerialStringReader.retrieve(); 
	CZ_LOG(logDefault, Log, "INPUT: %s", src);
}

void loop()
{
	CZ_LOG(logDefault, Log, F("millis=%u"), millis());
	tryReadString();
	delay(500);

	gScreen.setFont(NULL);
	gScreen.setCursor(0,0);
	gScreen.setTextColor(Colour_White, Colour_Black);
	gScreen.print(cz::formatString("Millis: %u", millis()));

	if (gTs.isTouched())
	{
		RawTouchPoint raw = gTs.getRawPoint();
		CZ_LOG(logDefault, Log, "Raw Data = (%u, %u, %u)", raw.x, raw.y, raw.z);

		TouchPoint p = gTs.getPoint();
		gScreen.setCursor(150, 150);
		gScreen.print(cz::formatString(" P=(%d, %d, %d) ", p.x, p.y, p.z));
		CZ_LOG(logDefault, Log, " P=(%d, %d, %d) ", p.x, p.y, p.z);
		gScreen.setBacklightBrightness(map(p.x, 0, 319, 0, 255));
	}
}
#endif

