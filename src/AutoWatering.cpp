#include "Config.h"

#if __MBED__
	#include "utility/PluggableUSBDevice_fix.h"
#endif

#include "Adafruit_MCP23017.h"
#include "utility/MCP23017Wrapper.h"
#include "crazygaze/micromuc/SerialStringReader.h"
#include "crazygaze/micromuc/Logging.h"
#include "crazygaze/micromuc/StringUtils.h"
#include "utility/MuxNChannels.h"
#include "gfx/MyDisplay1.h"
#include "gfx/MyXPT2046.h"

#if PORTING_TO_RP2040

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

#include "AT24C.h"
#include "crazygaze/micromuc/SDLogOutput.h"
#include "crazygaze/micromuc/Profiler.h"
#include "crazygaze/micromuc/SerialStringReader.h"
#include "MemorySetup.h"

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

SoilMoistureSensorTicker gSoilMoistureSensors[NUM_MOISTURESENSORS] =
{
	{true, 0, IO_EXPANDER_VPIN_SENSOR_0, MULTIPLEXER_MOISTURE_SENSOR_0}
#if NUM_MOISTURESENSORS>1
	,{true, 1, IO_EXPANDER_VPIN_SENSOR_1, MULTIPLEXER_MOISTURE_SENSOR_1}
#endif
#if NUM_MOISTURESENSORS>2
	,{true, 2, IO_EXPANDER_VPIN_SENSOR_2, MULTIPLEXER_MOISTURE_SENSOR_2}
#endif
#if NUM_MOISTURESENSORS>3
	,{true, 3, IO_EXPANDER_VPIN_SENSOR_3, MULTIPLEXER_MOISTURE_SENSOR_3}
#endif
};

using GroupMonitorTicker = TTicker<GroupMonitor, float, TickingMethod>;

GroupMonitorTicker gGroupMonitors[NUM_MOISTURESENSORS] =
{
	{ true, 0, IO_EXPANDER_MOTOR_0_INPUT1, IO_EXPANDER_MOTOR_0_INPUT2}
#if NUM_MOISTURESENSORS>1
	,{ true, 1, IO_EXPANDER_MOTOR_1_INPUT1, IO_EXPANDER_MOTOR_1_INPUT2}
#endif
#if NUM_MOISTURESENSORS>2
	,{ true, 2, IO_EXPANDER_MOTOR_2_INPUT1, IO_EXPANDER_MOTOR_2_INPUT2}
#endif
#if NUM_MOISTURESENSORS>3
	,{ true, 3, IO_EXPANDER_MOTOR_3_INPUT1, IO_EXPANDER_MOTOR_3_INPUT2}
#endif
};


TTicker<DisplayTFT, float, TickingMethod> gDisplay(true);

void doGroupShot(uint8_t index)
{
	gGroupMonitors[index].getObj().doShot();
}

unsigned long gPreviousMicros = 0;

struct Foo
{
	Foo(int a, int b, int c)
	: a(a), b(b), c(c)
	{
	}

	void log()
	{
		CZ_LOG(logDefault, Log, F("Foo(%d,%d,%d)"), a, b ,c);
	}

	int a,b,c;
};

FunctionTicker gMemLoggerFunc([]()
{
	logMemory();
}, 10.0f);

void setup()
{
#ifdef AVR8_BREAKPOINT_MODE
	debug_init();
	breakpoint();
#else
	// If using avr-stub, we can't use Serial
	Serial.begin(115200);
	while (!Serial) {
		; // wait for serial port to connect. Needed for native USB port only
	}
#endif

#if SD_CARD_LOGGING
	if (gSDCard.betin(SD_CARD_SS_PIN))
	{
		gSdLogOutput.begin(gSDCard.root, "log.txt", true);
		CZ_LOG(logDefault, Log, "SD card log file initialized");
	}
#endif

	setupMemoryAreas(2048);
	logMemory();

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

cz::SerialStringReader<64> gSerialStringReader;

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
				if (parseCommand(idx) && idx < NUM_MOISTURESENSORS)
				{
					gGroupMonitors[idx].getObj().turnMotorOff();
				}
			}
			else if (strcmp_P(cmd, (const char*)F("motoron"))==0)
			{
				int idx;
				if (parseCommand(idx) && idx < NUM_MOISTURESENSORS)
				{
					gGroupMonitors[idx].getObj().doShot();
				}
			}
			else if (strcmp_P(cmd, (const char*)F("setgroupthreshold"))==0)
			{
				int idx, value;
				if (parseCommand(idx, value) && idx < NUM_MOISTURESENSORS)
				{
					gCtx.data.getGroupData(idx).setThresholdValue(value);
				}
			}
			else if (strcmp_P(cmd, (const char*)F("startgroup"))==0)
			{
				int idx;
				if (parseCommand(idx) && idx < NUM_MOISTURESENSORS)
				{
					gCtx.data.getGroupData(idx).setRunning(true);
				}
			}
			else if (strcmp_P(cmd, (const char*)F("stopgroup"))==0)
			{
				int idx;
				if (parseCommand(idx) && idx < NUM_MOISTURESENSORS)
				{
					gCtx.data.getGroupData(idx).setRunning(false);
				}
			}
			else if (strcmp_P(cmd, (const char*)F("selectgroup"))==0)
			{
				int8_t idx;
				if (parseCommand(idx) && idx < NUM_MOISTURESENSORS)
				{
					gCtx.data.trySetSelectedGroup(idx);
				}
			}
			else if (strcmp_P(cmd, (const char*)F("setmocksensor"))==0)
			{
				int idx, value;
				if (parseCommand(idx, value) && idx < NUM_MOISTURESENSORS)
				{
					Component::raiseEvent(SetMockSensorValueEvent(idx, value));
				}
			}
			else if (strcmp_P(cmd, (const char*)F("setmocksensors"))==0)
			{
				int value;
				if (parseCommand(value))
				{
					for(int idx=0; idx<NUM_MOISTURESENSORS; idx++)
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
				if (parseCommand(idx) && idx < NUM_MOISTURESENSORS)
				{
					gCtx.data.saveGroupConfig(idx);
				}
			}
			else if (strcmp_P(cmd, (const char*)F("load"))==0)
			{
				gCtx.data.load();
			}
			else
			{
				CZ_LOG(logDefault, Error, F("Command \"%s\" not recognized"), gSerialStringReader.retrieve());
			}
		}
	}
	#endif // CONSOLE_COMMANDS

	gMemLoggerFunc.tick(deltaSeconds);

	gPreviousMicros = nowMicros;
}

#endif

#if 1

using namespace cz;
namespace
{
	cz::SerialLogOutput gSerialLogOutput;
	cz::SerialStringReader<> gSerialStringReader;
}

MyDisplay1 gTft(TFT_PIN_CS, TFT_PIN_DC, TFT_PIN_BACKLIGHT);
const TouchCalibrationData gTsCalibrationData = {211, 3412, 334, 3449, 4};
MyXPT2046 gTs(gTft, TOUCH_PIN_CS, TOUCH_PIN_IRQ, &gTsCalibrationData);

void setup()
{
	gSerialLogOutput.begin(Serial1, 115200);
	gSerialStringReader.begin(Serial1);

	Serial.print("Hello World-1!");
	Serial1.print("Hello World-2!");
	CZ_LOG(logDefault, Log, "Hello World-3!");
	CZ_LOG(logDefault, Log, F("Hello World-4!"));

	gTft.begin();
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

	gTft.setFont(NULL);
	gTft.setCursor(0,0);
	gTft.setTextColor(Colour_White, Colour_Black);
	gTft.print(cz::formatString("Millis: %u", millis()));

	if (gTs.isTouched())
	{
		RawTouchPoint raw = gTs.getRawPoint();
		CZ_LOG(logDefault, Log, "Raw Data = (%u, %u, %u)", raw.x, raw.y, raw.z);

		TouchPoint p = gTs.getPoint();
		gTft.setCursor(150, 150);
		gTft.print(cz::formatString(" P=(%d, %d, %d) ", p.x, p.y, p.z));
		CZ_LOG(logDefault, Log, " P=(%d, %d, %d) ", p.x, p.y, p.z);
		gTft.setBacklightBrightness(map(p.x, 0, 319, 0, 255));
	}
}
#endif

