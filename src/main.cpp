
#include <Arduino.h>

#ifdef AVR8_BREAKPOINT_MODE
#include "avr8-stub.h"
#endif

#include "Config.h"
#include "Context.h"
#include "SoilMoistureSensor.h"
#include "DisplayTFT.h"
#include "Utils.h"
#include "crazygaze/micromuc/Ticker.h"
#include "crazygaze/micromuc/Logging.h"
#include <algorithm>
#include <utility>

#include "AT24C.h"
#include "crazygaze/micromuc/SDLogOutput.h"
#include "crazygaze/micromuc/Profiler.h"
#include "crazygaze/micromuc/SerialStringReader.h"
#include "MemoryFree.h"

using namespace cz;

void operator delete(void* ptr, unsigned int size)
{
	free(ptr);
}


#if SD_CARD_LOGGING
	SDCardHelper gSDCard;
	SDLogOutput gSdLogOutput;
#endif

Context gCtx;

using SoilMoistureSensorTicker = TTicker<SoilMoistureSensor, float>;
SoilMoistureSensorTicker gSoilMoistureSensors[NUM_MOISTURESENSORS] =
{
	{true, gCtx, 0, IO_EXPANDER_VPIN_SENSOR_0, MULTIPLEXER_MOISTURE_SENSOR_0},
	{true, gCtx, 1, IO_EXPANDER_VPIN_SENSOR_1, MULTIPLEXER_MOISTURE_SENSOR_1},
	{true, gCtx, 2, IO_EXPANDER_VPIN_SENSOR_2, MULTIPLEXER_MOISTURE_SENSOR_2},
	{true, gCtx, 3, IO_EXPANDER_VPIN_SENSOR_3, MULTIPLEXER_MOISTURE_SENSOR_3}
};

TTicker<DisplayTFT, float> gDisplay(true, gCtx);

float gPreviousTime = 0;

void setMotorPins()
{
	gCtx.ioExpander.pinMode(IO_EXPANDER_MOTOR_0_INPUT1, OUTPUT);
	gCtx.ioExpander.pinMode(IO_EXPANDER_MOTOR_0_INPUT2, OUTPUT);
	gCtx.ioExpander.pinMode(IO_EXPANDER_MOTOR_1_INPUT1, OUTPUT);
	gCtx.ioExpander.pinMode(IO_EXPANDER_MOTOR_1_INPUT2, OUTPUT);
	gCtx.ioExpander.pinMode(IO_EXPANDER_MOTOR_2_INPUT1, OUTPUT);
	gCtx.ioExpander.pinMode(IO_EXPANDER_MOTOR_2_INPUT2, OUTPUT);
	gCtx.ioExpander.pinMode(IO_EXPANDER_MOTOR_3_INPUT1, OUTPUT);
	gCtx.ioExpander.pinMode(IO_EXPANDER_MOTOR_3_INPUT2, OUTPUT);

	gCtx.ioExpander.digitalWrite(IO_EXPANDER_MOTOR_0_INPUT1, LOW);
	gCtx.ioExpander.digitalWrite(IO_EXPANDER_MOTOR_0_INPUT2, LOW);
	gCtx.ioExpander.digitalWrite(IO_EXPANDER_MOTOR_1_INPUT1, LOW);
	gCtx.ioExpander.digitalWrite(IO_EXPANDER_MOTOR_1_INPUT2, LOW);
	gCtx.ioExpander.digitalWrite(IO_EXPANDER_MOTOR_2_INPUT1, LOW);
	gCtx.ioExpander.digitalWrite(IO_EXPANDER_MOTOR_2_INPUT2, LOW);
	gCtx.ioExpander.digitalWrite(IO_EXPANDER_MOTOR_3_INPUT1, LOW);
	gCtx.ioExpander.digitalWrite(IO_EXPANDER_MOTOR_3_INPUT2, LOW);
}

void setAllMotorPins(int val1, int val2)
{
	gCtx.ioExpander.digitalWrite(IO_EXPANDER_MOTOR_0_INPUT1, val1);
	gCtx.ioExpander.digitalWrite(IO_EXPANDER_MOTOR_0_INPUT2, val2);
	gCtx.ioExpander.digitalWrite(IO_EXPANDER_MOTOR_1_INPUT1, val1);
	gCtx.ioExpander.digitalWrite(IO_EXPANDER_MOTOR_1_INPUT2, val2);
	gCtx.ioExpander.digitalWrite(IO_EXPANDER_MOTOR_2_INPUT1, val1);
	gCtx.ioExpander.digitalWrite(IO_EXPANDER_MOTOR_2_INPUT2, val2);
	gCtx.ioExpander.digitalWrite(IO_EXPANDER_MOTOR_3_INPUT1, val1);
	gCtx.ioExpander.digitalWrite(IO_EXPANDER_MOTOR_3_INPUT2, val2);
}

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
	if (gSDCard.begin(SD_CARD_SS_PIN))
	{
		gSdLogOutput.begin(gSDCard.root, "log.txt", true);
		CZ_LOG(logDefault, Log, "SD card log file initialized");
	}
#endif

	gCtx.begin();
	gDisplay.getObj().begin();

	for(auto&& ticker : gSoilMoistureSensors)
	{
		ticker.getObj().begin();
	}

	gPreviousTime = millis() / 1000.0f;

	setMotorPins();

	//setAllMotorPins(HIGH, LOW);
	//delay(1000);
	//setAllMotorPins(LOW, LOW);
	//delay(3000);

	//gCtx.ioExpander.digitalWrite(IO_EXPANDER_MOTOR_2_INPUT1, HIGH);
	//gCtx.ioExpander.digitalWrite(IO_EXPANDER_MOTOR_2_INPUT2, LOW);
	//delay(3000);
	//gCtx.ioExpander.digitalWrite(IO_EXPANDER_MOTOR_2_INPUT1, LOW);
	//gCtx.ioExpander.digitalWrite(IO_EXPANDER_MOTOR_2_INPUT2, HIGH);
	//delay(3000);
	//gCtx.ioExpander.digitalWrite(IO_EXPANDER_MOTOR_2_INPUT1, LOW);
	//gCtx.ioExpander.digitalWrite(IO_EXPANDER_MOTOR_2_INPUT2, LOW);

}

PROFILER_CREATE(10);

cz::SerialStringReader<64> gSerialStringReader;

void loop()
{
	PROFILER_STARTRUN();

	float now = millis() / 1000.0f;
	float deltaSeconds = now - gPreviousTime;
	float countdown = 60*60;
	
	{
		PROFILE_SCOPE(F("TickAll"));
		countdown = std::min(gDisplay.tick(deltaSeconds), countdown);

		for (auto&& ticker : gSoilMoistureSensors)
		{
			countdown = std::min(ticker.tick(deltaSeconds), countdown);
		}

	}


	// We use this so that we can just put a breakpoint in here and force the code to run when we want to check the profiler data
	if (CZ_PROFILER || true)
	{
		if (gSerialStringReader.tryRead())
		{
			if (strcmp_P(gSerialStringReader.retrieve(), (const char*)F("profiler_log"))==0)
			{
				PROFILER_LOG();
			}
			else
			{
				CZ_LOG(logDefault, Error, F("Command \"%s\" not recognized"), gSerialStringReader.retrieve());
			}
		}
	}

	//gCtx.data.logMoistureSensors();

	gPreviousTime = now;
	//delay(countdown*1000);
}

