#include "Config/Config.h"

#if __MBED__
	#include "utility/PluggableUSBDevice_fix.h"
#endif

#include "Context.h"
#include "SoilMoistureSensor.h"
#include "GroupMonitor.h"
#include "Timer.h"
#include "crazygaze/micromuc/Logging.h"
#include <algorithm>
#include <utility>
#include <memory>

#include "crazygaze/micromuc/SDLogOutput.h"
#include "crazygaze/micromuc/Profiler.h"
#include "gfx/MyDisplay1.h"
#include "crazygaze/TouchController/XPT2046.h"

using namespace cz;

#if SD_CARD_LOGGING
	SDCardHelper gSDCard;
	SDLogOutput gSdLogOutput;
#endif

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

#if CONSOLE_COMMANDS_ENABLED
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

	// We need to increase the uart queue size so CommandConsole works with long commands.
	// Couldn't pinpoint the exact reason, but at the time of writting, SerialStringReader would messe up when trying to read long commands
	MySerial.setFIFOSize(256);

	#if CZ_USE_PROBE_SERIAL
		MySerial.setRX(MySerial_RXPin);
		MySerial.setTX(MySerial_TXPin);
	#endif
	gSerialLogOutput.begin(MySerial, 115200);
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
	if (gSDCard.begin(SD_CARD_SS_PIN))
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

	{
		unsigned long ms = static_cast<unsigned long>(countdown * 1000);
		delay(ms);
	}

}
