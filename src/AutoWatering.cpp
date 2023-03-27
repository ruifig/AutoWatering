#include "Config.h"

#if __MBED__
	#include "utility/PluggableUSBDevice_fix.h"
#endif

#include "Config.h"
#include "Context.h"
#include "TemperatureAndHumiditySensor.h"
#include "SoilMoistureSensor.h"
#include "GroupMonitor.h"
#include "DisplayTFT.h"
#include "Timer.h"
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

using TemperatureAndHumiditySensorTicker = TTicker<TemperatureAndHumiditySensor, float, TickingMethod>;
TemperatureAndHumiditySensorTicker gTempAndHumiditySensor(true);

#define SOILMOISTURE_TICKER(index, boardIndex, POWER_PIN, MUXPIN) \
	 {true, index, IOExpanderPinInstance(gCtx.m_i2cBoards[boardIndex].ioExpander, POWER_PIN), MuxPinInstance(gCtx.m_i2cBoards[boardIndex].mux, MUXPIN)}

SoilMoistureSensorTicker gSoilMoistureSensors[MAX_NUM_PAIRS] =
{
	//
	// First i2c board
	//
	 //{true, 0, IOExpanderPinInstance(gCtx.m_i2cBoards[0].ioExpander, IO_EXPANDER_VPIN_SENSOR0), MuxPinInstance(gCtx.m_i2cBoards[0].mux, MUX_MOISTURE_SENSOR0)}
	 SOILMOISTURE_TICKER(0, 0, IO_EXPANDER_VPIN_SENSOR0, MUX_MOISTURE_SENSOR0)
#if MAX_NUM_PAIRS>1
	,SOILMOISTURE_TICKER(1, 0, IO_EXPANDER_VPIN_SENSOR1, MUX_MOISTURE_SENSOR1)
#endif
#if MAX_NUM_PAIRS>2
	,SOILMOISTURE_TICKER(2, 0, IO_EXPANDER_VPIN_SENSOR2, MUX_MOISTURE_SENSOR2)
#endif
#if MAX_NUM_PAIRS>3
	,SOILMOISTURE_TICKER(3, 0, IO_EXPANDER_VPIN_SENSOR3, MUX_MOISTURE_SENSOR3)
#endif
#if MAX_NUM_PAIRS>4
	,SOILMOISTURE_TICKER(4, 0, IO_EXPANDER_VPIN_SENSOR4, MUX_MOISTURE_SENSOR4)
#endif
#if MAX_NUM_PAIRS>5
	,SOILMOISTURE_TICKER(5, 0, IO_EXPANDER_VPIN_SENSOR5, MUX_MOISTURE_SENSOR5)
#endif

	//
	// Second i2c board
	//
#if MAX_NUM_PAIRS>6
	,SOILMOISTURE_TICKER(6, 1, IO_EXPANDER_VPIN_SENSOR0, MUX_MOISTURE_SENSOR0)
#endif
#if MAX_NUM_PAIRS>7
	,SOILMOISTURE_TICKER(7, 1, IO_EXPANDER_VPIN_SENSOR1, MUX_MOISTURE_SENSOR1)
#endif
#if MAX_NUM_PAIRS>8
	,SOILMOISTURE_TICKER(8, 1, IO_EXPANDER_VPIN_SENSOR2, MUX_MOISTURE_SENSOR2)
#endif
#if MAX_NUM_PAIRS>9
	,SOILMOISTURE_TICKER(9, 1, IO_EXPANDER_VPIN_SENSOR3, MUX_MOISTURE_SENSOR3)
#endif
#if MAX_NUM_PAIRS>10
	,SOILMOISTURE_TICKER(10, 1, IO_EXPANDER_VPIN_SENSOR4, MUX_MOISTURE_SENSOR4)
#endif
#if MAX_NUM_PAIRS>11
	,SOILMOISTURE_TICKER(11, 1, IO_EXPANDER_VPIN_SENSOR5, MUX_MOISTURE_SENSOR5)
#endif

};

using GroupMonitorTicker = TTicker<GroupMonitor, float, TickingMethod>;

GroupMonitorTicker gGroupMonitors[MAX_NUM_PAIRS] =
{
	//
	// First i2c board
	//
	 { true, 0, IOExpanderPinInstance(gCtx.m_i2cBoards[0].ioExpander, IO_EXPANDER_MOTOR0)}
#if MAX_NUM_PAIRS>1
	,{ true, 1, IOExpanderPinInstance(gCtx.m_i2cBoards[0].ioExpander, IO_EXPANDER_MOTOR1)}
#endif
#if MAX_NUM_PAIRS>2
	,{ true, 2, IOExpanderPinInstance(gCtx.m_i2cBoards[0].ioExpander, IO_EXPANDER_MOTOR2)}
#endif
#if MAX_NUM_PAIRS>3
	,{ true, 3, IOExpanderPinInstance(gCtx.m_i2cBoards[0].ioExpander, IO_EXPANDER_MOTOR3)}
#endif
#if MAX_NUM_PAIRS>4
	,{ true, 4, IOExpanderPinInstance(gCtx.m_i2cBoards[0].ioExpander, IO_EXPANDER_MOTOR4)}
#endif
#if MAX_NUM_PAIRS>5
	,{ true, 5, IOExpanderPinInstance(gCtx.m_i2cBoards[0].ioExpander, IO_EXPANDER_MOTOR5)}
#endif

//
// Second i2c board
//
#if MAX_NUM_PAIRS>6
	,{ true, 6, IOExpanderPinInstance(gCtx.m_i2cBoards[1].ioExpander, IO_EXPANDER_MOTOR0)}
#endif
#if MAX_NUM_PAIRS>7
	,{ true, 7, IOExpanderPinInstance(gCtx.m_i2cBoards[1].ioExpander, IO_EXPANDER_MOTOR1)}
#endif
#if MAX_NUM_PAIRS>8
	,{ true, 8, IOExpanderPinInstance(gCtx.m_i2cBoards[1].ioExpander, IO_EXPANDER_MOTOR2)}
#endif
#if MAX_NUM_PAIRS>9
	,{ true, 9, IOExpanderPinInstance(gCtx.m_i2cBoards[1].ioExpander, IO_EXPANDER_MOTOR3)}
#endif
#if MAX_NUM_PAIRS>10
	,{ true, 10, IOExpanderPinInstance(gCtx.m_i2cBoards[1].ioExpander, IO_EXPANDER_MOTOR4)}
#endif
#if MAX_NUM_PAIRS>11
	,{ true, 11, IOExpanderPinInstance(gCtx.m_i2cBoards[1].ioExpander, IO_EXPANDER_MOTOR5)}
#endif

};


namespace cz
{
	MyDisplay1 gScreen;
	bool gScreenDetected = false;
	const TouchCalibrationData gTsCalibrationData = {233, 3460, 349, 3547, 2};
	// Minimal graphics interface the touch controller needs
	class TouchControllerDisplayWrapper : public TouchScreenController_GraphicsInterface
	{
		public:
		virtual int16_t width() override { return gScreen.width(); }
		virtual int16_t height() override { return gScreen.height(); }
		virtual void fillScreen(uint16_t colour) override { gScreen.fillScreen(Colour(colour)); }
		virtual void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t colour) override
		{
			gScreen.drawLine(x0, y0, x1, y1, Colour(colour));
		}
	};
	TouchControllerDisplayWrapper gTsDisplayWrapper;

#ifdef TOUCH_MY_IRQ
	#define Touch_Pin_IRQ TOUCH_MY_IRQ
#else
	#define Touch_Pin_IRQ \
		255  // The touch library checks for a value of 255, which means it won't try to use an irq pin
#endif
	XPT2046 gTs(gTsDisplayWrapper, TOUCH_MY_CS, Touch_Pin_IRQ, &gTsCalibrationData);

	Timer gTimer;
}

// TFT_espi spi object
// Need access to this, so I can reset the frequency after using touch
extern "C"
{
	extern SPIClassRP2040 spi;
}

namespace
{
#if CZ_SERIAL_LOG_ENABLED
	cz::SerialLogOutput gSerialLogOutput;
#endif

#if CONSOLE_COMMANDS
	cz::SerialStringReader<> gSerialStringReader;
#endif
}

TTicker<DisplayTFT, float, TickingMethod> gDisplay(true);

void doGroupShot(uint8_t index)
{
	gGroupMonitors[index].getObj().doShot();
}

void setup()
{
	// MCP23017 : 400kHz at 3.3v
	// AT24C256 : 400kHz at 2.7v, 2.5v
	Wire.setSDA(0);
	Wire.setSCL(1);
	Wire.begin();
	Wire.setClock(400000);

	#if MOCK_COMPONENTS
		randomSeed(micros());
	#endif

#if CZ_SERIAL_LOG_ENABLED

	#if CZ_USE_PROBE_SERIAL
		MySerial.setRX(MySerial_RXPin);
		MySerial.setTX(MySerial_TXPin);
	#endif

	gSerialLogOutput.begin(MySerial, 115200);
#endif

#if CONSOLE_COMMANDS
	gSerialStringReader.begin(MySerial);
#endif

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

	if (gScreen.begin())
	{
		gScreenDetected = true;
		CZ_LOG(logDefault, Log, "Screen detected.");
	}
	else
	{
		gScreenDetected = false;
		CZ_LOG(logDefault, Warning, "*** SCREEN NOT DETECTED ***");
	}

	gTs.begin(spi);
	//gTs.calibrate();

	gCtx.begin();
	gDisplay.getObj().begin();

	gTempAndHumiditySensor.getObj().begin();

	for(auto&& ticker : gSoilMoistureSensors)
	{
		ticker.getObj().begin();
	}

	for(auto&& ticker : gGroupMonitors)
	{
		ticker.getObj().begin();
	}

	gTimer.begin();

	//runTests(gCtx.eeprom);
}

PROFILER_CREATE(30);

uint32_t gTickCount = 0;

void loop()
{
	PROFILER_STARTRUN();
	gTickCount++;

	gTimer.update();
	const float deltaSeconds = gTimer.getDeltaSeconds();

	float countdown = 60*60;
	{
		PROFILE_SCOPE(F("TickAll"));
		countdown = std::min(gDisplay.tick(deltaSeconds), countdown);

		countdown = std::min(gTempAndHumiditySensor.tick(deltaSeconds), countdown);

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
		while (gSerialStringReader.tryRead())
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
			else if (strcmp_P(cmd, (const char*)F("scroll"))==0)
			{
				int inc;
				if (parseCommand(inc))
				{
					gDisplay.getObj().scrollSlots(inc);
				}
			}
			else if (strcmp_P(cmd, (const char*)F("motoroff"))==0)
			{
				int idx;
				if (parseCommand(idx) && idx < MAX_NUM_PAIRS)
				{
					gGroupMonitors[idx].getObj().turnMotorOff();
				}
			}
			else if (strcmp_P(cmd, (const char*)F("motoron"))==0)
			{
				int idx;
				if (parseCommand(idx) && idx < MAX_NUM_PAIRS)
				{
					gGroupMonitors[idx].getObj().doShot();
				}
			}
			else if (strcmp_P(cmd, (const char*)F("setmuxenabled"))==0)
			{
				int boardIdx;
				bool enabled;
				if (parseCommand(boardIdx, enabled))
				{
					if (boardIdx>=0 && boardIdx<MAX_NUM_I2C_BOARDS)
					{
						gCtx.m_i2cBoards[boardIdx].mux.setEnabled(enabled);
					}
					else
					{
						CZ_LOG(logDefault, Error, F("Invalid board index (%d)"), boardIdx);
					}
				}
			}
			else if (strcmp_P(cmd, (const char*)F("setmuxchannel"))==0)
			{
				int boardIdx;
				int muxpin;
				if (parseCommand(boardIdx, muxpin))
				{
					if (boardIdx<0 && boardIdx>=MAX_NUM_I2C_BOARDS)
					{
						CZ_LOG(logDefault, Error, F("Invalid board index (%d)"), boardIdx);
						continue;
					}

					if (muxpin<0 || muxpin>=8)
					{
						CZ_LOG(logDefault, Error, F("Invalid mux channel (%d)"), muxpin);
						continue;
					}

					gCtx.m_i2cBoards[boardIdx].mux.setChannel(MultiplexerPin(muxpin));
				}
			}
			else if (strcmp_P(cmd, (const char*)F("setgroupthreshold"))==0)
			{
				int idx, value;
				if (parseCommand(idx, value) && idx < MAX_NUM_PAIRS)
				{
					gCtx.data.getGroupData(idx).setThresholdValue(value);
				}
			}
			else if (strcmp_P(cmd, (const char*)F("setgroupthresholdaspercentage"))==0)
			{
				int idx, value;
				if (parseCommand(idx, value) && idx < MAX_NUM_PAIRS)
				{
					gCtx.data.getGroupData(idx).setThresholdValueAsPercentage(value);
				}
			}
			else if (strcmp_P(cmd, (const char*)F("startgroup"))==0)
			{
				int idx;
				if (parseCommand(idx) && idx < MAX_NUM_PAIRS)
				{
					gCtx.data.getGroupData(idx).setRunning(true);
				}
			}
			else if (strcmp_P(cmd, (const char*)F("stopgroup"))==0)
			{
				int idx;
				if (parseCommand(idx) && idx < MAX_NUM_PAIRS)
				{
					gCtx.data.getGroupData(idx).setRunning(false);
				}
			}
			else if (strcmp_P(cmd, (const char*)F("logconfig"))==0)
			{
				gCtx.data.logConfig();
			}
			else if (strcmp_P(cmd, (const char*)F("loggroupconfig"))==0)
			{
				int idx;
				if (parseCommand(idx) && idx < MAX_NUM_PAIRS)
				{
					gCtx.data.getGroupData(idx).logConfig();
				}
			}
			else if (strcmp_P(cmd, (const char*)F("selectgroup"))==0)
			{
				int8_t idx;
				if (parseCommand(idx) && idx < MAX_NUM_PAIRS)
				{
					gCtx.data.trySetSelectedGroup(idx);
				}
			}
			else if (strcmp_P(cmd, (const char*)F("setmocksensorerrorstatus"))==0)
			{
				int idx, status;
				if (parseCommand(idx, status))
				{
					if (status>=SensorReading::Status::First && status<=SensorReading::Status::Last)
					{
						Component::raiseEvent(SetMockSensorErrorStatusEvent(idx, static_cast<SensorReading::Status>(status)));
					}
					else
					{
						CZ_LOG(logDefault, Error, F("Invalid status value"));
					}
				}
			}
			else if (strcmp_P(cmd, (const char*)F("setmocksensor"))==0)
			{
				int idx, value;
				if (parseCommand(idx, value) && idx < MAX_NUM_PAIRS)
				{
					Component::raiseEvent(SetMockSensorValueEvent(idx, value));
				}
			}
			else if (strcmp_P(cmd, (const char*)F("setmocksensors"))==0)
			{
				int value;
				if (parseCommand(value))
				{
					for(int idx=0; idx<MAX_NUM_PAIRS; idx++)
					{
						Component::raiseEvent(SetMockSensorValueEvent(idx, value));
					}
				}
			}
			else if (strcmp_P(cmd, (const char*)F("save"))==0)
			{
				gCtx.data.save();

				ProgramData prgData(gCtx);
				prgData.begin();
				prgData.load();
				prgData.logConfig();
			}
			else if (strcmp_P(cmd, (const char*)F("savegroup"))==0)
			{
				uint8_t idx;
				if (parseCommand(idx) && idx < MAX_NUM_PAIRS)
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

}
