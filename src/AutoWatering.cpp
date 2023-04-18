#include "Config/Config.h"

#if __MBED__
	#include "utility/PluggableUSBDevice_fix.h"
#endif

#include "Context.h"
#include "SoilMoistureSensor.h"
#include "GroupMonitor.h"
#include "GraphicalUI.h"
#include "AdafruitIOManager.h"
#include "Timer.h"
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

// #TODO : Remove these
struct SemaphoreDebug
{
	struct Take
	{
		const char* file;
		int line;
		int counter;
	} take;

	struct Give
	{
		const char* file;
		int line;
		int counter;
	} give;

	int counter;
} gSem;

extern void *pxCurrentTCB;

void MySemaphoreTake(bool& sem, const char* file, int line)
{
	while(sem)
	{
	}
	sem = true;
	++gSem.counter;
	++gSem.take.counter;
	gSem.take.file = file;
	gSem.take.line = line;
}

void MySemaphoreGive(bool& sem, const char* file, int line)
{
	gSem.give.file = file;
	gSem.give.line = line;
	++gSem.give.counter;
	--gSem.counter;
	sem = false;
}

#define SOILMOISTURE_TICKER(index, boardIndex, POWER_PIN, MUXPIN) \
	 {index, IOExpanderPinInstance(gCtx.m_i2cBoards[boardIndex].ioExpander, POWER_PIN), MuxPinInstance(gCtx.m_i2cBoards[boardIndex].mux, MUXPIN)}

SoilMoistureSensor gSoilMoistureSensors[MAX_NUM_PAIRS] =
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

GroupMonitor gGroupMonitors[MAX_NUM_PAIRS] =
{
	//
	// First i2c board
	//
	 {0, IOExpanderPinInstance(gCtx.m_i2cBoards[0].ioExpander, IO_EXPANDER_MOTOR0)}
#if MAX_NUM_PAIRS>1
	,{1, IOExpanderPinInstance(gCtx.m_i2cBoards[0].ioExpander, IO_EXPANDER_MOTOR1)}
#endif
#if MAX_NUM_PAIRS>2
	,{2, IOExpanderPinInstance(gCtx.m_i2cBoards[0].ioExpander, IO_EXPANDER_MOTOR2)}
#endif
#if MAX_NUM_PAIRS>3
	,{3, IOExpanderPinInstance(gCtx.m_i2cBoards[0].ioExpander, IO_EXPANDER_MOTOR3)}
#endif
#if MAX_NUM_PAIRS>4
	,{4, IOExpanderPinInstance(gCtx.m_i2cBoards[0].ioExpander, IO_EXPANDER_MOTOR4)}
#endif
#if MAX_NUM_PAIRS>5
	,{5, IOExpanderPinInstance(gCtx.m_i2cBoards[0].ioExpander, IO_EXPANDER_MOTOR5)}
#endif

//
// Second i2c board
//
#if MAX_NUM_PAIRS>6
	,{ 6, IOExpanderPinInstance(gCtx.m_i2cBoards[1].ioExpander, IO_EXPANDER_MOTOR0)}
#endif
#if MAX_NUM_PAIRS>7
	,{ 7, IOExpanderPinInstance(gCtx.m_i2cBoards[1].ioExpander, IO_EXPANDER_MOTOR1)}
#endif
#if MAX_NUM_PAIRS>8
	,{ 8, IOExpanderPinInstance(gCtx.m_i2cBoards[1].ioExpander, IO_EXPANDER_MOTOR2)}
#endif
#if MAX_NUM_PAIRS>9
	,{ 9, IOExpanderPinInstance(gCtx.m_i2cBoards[1].ioExpander, IO_EXPANDER_MOTOR3)}
#endif
#if MAX_NUM_PAIRS>10
	,{ 10, IOExpanderPinInstance(gCtx.m_i2cBoards[1].ioExpander, IO_EXPANDER_MOTOR4)}
#endif
#if MAX_NUM_PAIRS>11
	,{ 11, IOExpanderPinInstance(gCtx.m_i2cBoards[1].ioExpander, IO_EXPANDER_MOTOR5)}
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

void doGroupShot(uint8_t index)
{
	gGroupMonitors[index].doShot();
}

void setup()
{
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

	CZ_LOG(logDefault, Log, "");
	CZ_LOG(logDefault, Log, "Autowatering, %s", __TIMESTAMP__);

	CZ_LOG(logDefault, Log, "Setting ADC reporting to %d bits", ADC_NUM_BITS);
	analogReadResolution(ADC_NUM_BITS);

	CZ_LOG(logDefault, Log, "Initializing I2C");
	// MCP23017 : 400kHz at 3.3v
	// AT24C256 : 400kHz at 2.7v, 2.5v
	Wire.setSDA(0);
	Wire.setSCL(1);
	Wire.begin();
	Wire.setClock(400000);
	//
	// Initialize I2C
	// NOTE: The original Adafruit_MCP23017.cpp code automatically initialized IC2, but that causes some problems
	// because I also need to set the clock before I2C is used, and also because eeprom is also using I2C.
	// So, Adafruit_MCP23017 has a change to disable I2C initialization, and initialization is done here.
	Wire.begin();
	// MCP23017 : 400kHz at 3.3v
	// AT24C256 : 400kHz at 2.7v, 2.5v
	Wire.setClock(400000);

	#if MOCK_COMPONENTS
		randomSeed(micros());
	#endif

#if SD_CARD_LOGGING
	if (gSDCard.betin(SD_CARD_SS_PIN))
	{
		gSdLogOutput.begin(gSDCard.root, "log.txt", true);
		CZ_LOG(logDefault, Log, "SD card log file initialized");
	}
#endif

	CZ_LOG(logDefault, Log, "Initialing screen");
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

	if (gScreenDetected)
	{
		CZ_LOG(logDefault, Log, "Initializing touchscreen");
		gTs.begin(spi);
		CZ_LOG(logDefault, Log, "Finished initializing touchscreen");
		//gTs.calibrate();
	}

	gCtx.begin();
	gTimer.begin();
	Component::initAll();

	CZ_LOG(logDefault, Log, "Finished setup()");
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
		countdown = std::min(Component::tickAll(deltaSeconds), countdown);
	}

	// We use this so that we can just put a breakpoint in here and force the code to run when we want to check the profiler data
	#if CONSOLE_COMMANDS
	{
		while (gSerialStringReader.tryRead())
		{
			Command cmd(gSerialStringReader.retrieve());
			if (!cmd.parseCmd())
			{
				continue;
			}

			if (cmd.targetComponent)
			{
				if (!cmd.targetComponent->processCommand(cmd))
				{
					CZ_LOG(logDefault, Error, "Failed to execute %s.%s command", cmd.targetComponent->getName(), cmd.cmd);
				}
			}
			else if (strcasecmp_P(cmd.cmd, (const char*)F("profiler_log"))==0)
			{
				PROFILER_LOG();
			}
			else if (strcasecmp_P(cmd.cmd, (const char*)F("profiler_reset"))==0)
			{
				PROFILER_RESET();
			}
			else if (strcasecmp_P(cmd.cmd, (const char*)F("heapinfo"))==0)
			{
				CZ_LOG(logDefault, Log,"HEAP INFO: size=%d, used=%d, free=%d", rp2040.getTotalHeap(), rp2040.getUsedHeap(), rp2040.getFreeHeap());
			}
			else if (strcasecmp_P(cmd.cmd, (const char*)F("scroll"))==0)
			{
				int inc;
				if (cmd.parseParams(inc))
				{
					gGraphicalUI.scrollSlots(inc);
				}
			}
			else if (strcasecmp_P(cmd.cmd, (const char*)F("setmuxenabled"))==0)
			{
				int boardIdx;
				bool enabled;
				if (cmd.parseParams(boardIdx, enabled))
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
			else if (strcasecmp_P(cmd.cmd, (const char*)F("setmuxchannel"))==0)
			{
				int boardIdx;
				int muxpin;
				if (cmd.parseParams(boardIdx, muxpin))
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
			else if (strcasecmp_P(cmd.cmd, (const char*)F("setgroupthreshold"))==0)
			{
				int idx, value;
				if (cmd.parseParams(idx, value) && idx < MAX_NUM_PAIRS)
				{
					gCtx.data.getGroupData(idx).setThresholdValue(value);
				}
			}
			else if (strcasecmp_P(cmd.cmd, (const char*)F("setgroupthresholdaspercentage"))==0)
			{
				int idx, value;
				if (cmd.parseParams(idx, value) && idx < MAX_NUM_PAIRS)
				{
					gCtx.data.getGroupData(idx).setThresholdValueAsPercentage(value);
				}
			}
			else if (strcasecmp_P(cmd.cmd, (const char*)F("startgroup"))==0)
			{
				int idx;
				if (cmd.parseParams(idx) && idx < MAX_NUM_PAIRS)
				{
					gCtx.data.getGroupData(idx).setRunning(true);
				}
			}
			else if (strcasecmp_P(cmd.cmd, (const char*)F("stopgroup"))==0)
			{
				int idx;
				if (cmd.parseParams(idx) && idx < MAX_NUM_PAIRS)
				{
					gCtx.data.getGroupData(idx).setRunning(false);
				}
			}
			else if (strcasecmp_P(cmd.cmd, (const char*)F("logconfig"))==0)
			{
				gCtx.data.logConfig();
			}
			else if (strcasecmp_P(cmd.cmd, (const char*)F("loggroupconfig"))==0)
			{
				int idx;
				if (cmd.parseParams(idx) && idx < MAX_NUM_PAIRS)
				{
					gCtx.data.getGroupData(idx).logConfig();
				}
			}
			else if (strcasecmp_P(cmd.cmd, (const char*)F("selectgroup"))==0)
			{
				int8_t idx;
				if (cmd.parseParams(idx) && idx < MAX_NUM_PAIRS)
				{
					gCtx.data.trySetSelectedGroup(idx);
				}
			}
			else if (strcasecmp_P(cmd.cmd, (const char*)F("setmocksensorerrorstatus"))==0)
			{
				int idx, status;
				if (cmd.parseParams(idx, status))
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
			else if (strcasecmp_P(cmd.cmd, (const char*)F("setmocksensor"))==0)
			{
				int idx, value;
				if (cmd.parseParams(idx, value) && idx < MAX_NUM_PAIRS)
				{
					Component::raiseEvent(SetMockSensorValueEvent(idx, value));
				}
			}
			else if (strcasecmp_P(cmd.cmd, (const char*)F("setmocksensors"))==0)
			{
				int value;
				if (cmd.parseParams(value))
				{
					for(int idx=0; idx<MAX_NUM_PAIRS; idx++)
					{
						Component::raiseEvent(SetMockSensorValueEvent(idx, value));
					}
				}
			}
			else if (strcasecmp_P(cmd.cmd, (const char*)F("save"))==0)
			{
				gCtx.data.save();

				ProgramData prgData(gCtx);
				prgData.begin();
				prgData.load();
				prgData.logConfig();
			}
			else if (strcasecmp_P(cmd.cmd, (const char*)F("savegroup"))==0)
			{
				uint8_t idx;
				if (cmd.parseParams(idx) && idx < MAX_NUM_PAIRS)
				{
					gCtx.data.saveGroupConfig(idx);
				}
			}
			else if (strcasecmp_P(cmd.cmd, (const char*)F("load"))==0)
			{
				gCtx.data.load();
			}
			else if (strcasecmp_P(cmd.cmd, (const char*)F("setverbosity"))==0)
			{
				char name[30];
				int verbosity;
				constexpr int minVerbosity = static_cast<int>(LogVerbosity::Fatal);
				constexpr int maxVerbosity = static_cast<int>(LogVerbosity::Verbose);
				if (cmd.parseParams(name, verbosity))
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
			else if (strcasecmp_P(cmd.cmd, (const char*)F("delay"))==0) // Blocks for X ms. Good to simulate a freeze to test the watchdog
			{
				int ms;
				if (cmd.parseParams(ms))
				{
					CZ_LOG(logDefault, Log, "Delay(%d)", ms);
					delay(ms);
				}
			}
			#if WIFI_ENABLED
			else if (strcasecmp_P(cmd.cmd, (const char*)F("logcachedmqttvalues"))==0)
			{
				gAdafruitIOManager.logCache();
			}
			#endif
			else
			{
				CZ_LOG(logDefault, Error, F("Command \"%s\" not recognized"), gSerialStringReader.retrieve());
			}
		}
	}
	#endif // CONSOLE_COMMANDS


	{
		unsigned long ms = static_cast<unsigned long>(countdown * 1000);
		delay(ms);
	}

}
