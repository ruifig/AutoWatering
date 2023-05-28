#if __MBED__
	#include "utility/PluggableUSBDevice_fix.h"
#endif

#include "Context.h"
#include "SoilMoistureSensor.h"
#include "PumpMonitor.h"
#include "Timer.h"
#include "crazygaze/micromuc/Logging.h"
#include <algorithm>
#include <utility>
#include <memory>

#include "crazygaze/micromuc/SDLogOutput.h"
#include "crazygaze/micromuc/Profiler.h"
#include "gfx/TFTeSPIWrapper.h"
#include "crazygaze/TouchController/XPT2046.h"
#include <memory>
#include <vector>

using namespace cz;

#if AW_SD_CARD_LOGGING
	SDCardHelper gSDCard;
	SDLogOutput gSdLogOutput;
#endif


namespace cz
{
	Setup* gSetup;
}

namespace cz
{
#if AW_TOUCHUI_ENABLED
	const TouchCalibrationData gTsCalibrationData = {233, 3460, 349, 3547, 2};
	// Minimal graphics interface the touch controller needs
	class TouchControllerDisplayWrapper : public TouchScreenController_GraphicsInterface
	{
		public:
		virtual int16_t width() override { return TFTeSPIWrapper::getInstance()->width(); }
		virtual int16_t height() override { return TFTeSPIWrapper::getInstance()->height(); }
		virtual void fillScreen(uint16_t colour) override { TFTeSPIWrapper::getInstance()->fillScreen(Colour(colour)); }
		virtual void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t colour) override
		{
			TFTeSPIWrapper::getInstance()->drawLine(x0, y0, x1, y1, Colour(colour));
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
#endif

	Timer gTimer;
}

// TFT_espi spi object
// Need access to this, so I can reset the frequency after using touch
#if AW_SPI_ENABLED
extern "C"
{
	extern SPIClassRP2040 spi;
}
#endif

namespace
{
#if CZ_SERIAL_LOG_ENABLED
	cz::SerialLogOutput gSerialLogOutput;
#endif

#if CONSOLE_COMMANDS_ENABLED
	cz::SerialStringReader<> gSerialStringReader;
#endif
}

void setup()
{
#if CZ_SERIAL_LOG_ENABLED

	// We need to increase the uart queue size so CommandConsole works with long commands.
	// Couldn't pinpoint the exact reason, but at the time of writting, SerialStringReader would messe up when trying to read long commands
	AW_CUSTOM_SERIAL.setFIFOSize(256);

	#if defined(AW_CUSTOM_SERIAL_RXPIN) || defined(AW_CUSTOM_SERIAL_TXPIN)
		AW_CUSTOM_SERIAL.setRX(AW_CUSTOM_SERIAL_RXPIN);
		AW_CUSTOM_SERIAL.setTX(AW_CUSTOM_SERIAL_TXPIN);
	#endif
	gSerialLogOutput.begin(AW_CUSTOM_SERIAL, 115200);
#endif

	CZ_LOG(logDefault, Log, "");
	CZ_LOG(logDefault, Log, "Autowatering, %s", __TIMESTAMP__);

	CZ_LOG(logDefault, Log, "Setting ADC reporting to %d bits", AW_ADC_NUM_BITS);
	analogReadResolution(AW_ADC_NUM_BITS);

	CZ_LOG(logDefault, Log, "Initializing I2C");

#if AW_I2C_ENABLED
		Wire.setSDA(AW_I2C_SDAPIN);
		Wire.setSCL(AW_I2C_SCLPIN);
		Wire.begin();
		Wire.setClock(AW_I2C_SPEEDHZ);
		//
		// Initialize I2C
		// NOTE: The original Adafruit_MCP23017.cpp code automatically initialized IC2, but that causes some problems
		// because I also need to set the clock before I2C is used, and also because eeprom is also using I2C.
		// So, Adafruit_MCP23017 has a change to disable I2C initialization, and initialization is done here.
		Wire.begin();
#endif
	

	#if AW_MOCK_COMPONENTS
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

#if AW_TOUCHUI_ENABLED
	TFTeSPIWrapper::getInstance()->begin();

	CZ_LOG(logDefault, Log, "Initializing touchscreen");
	gTs.begin(spi);
	CZ_LOG(logDefault, Log, "Finished initializing touchscreen");
	//gTs.calibrate();
#endif

	gCtx.begin();
	gTimer.begin();

	gSetup = createSetupObject();
	gSetup->begin();
	gSetup->createSoilMoistureSensors();
	gSetup->createPumpMonitors();

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
		countdown = std::min(Component::tickAll(deltaSeconds), countdown);
	}

	{
		unsigned long ms = static_cast<unsigned long>(countdown * 1000);
		delay(ms);
	}

}
