#include <Arduino.h>

#ifdef AVR8_BREAKPOINT_MODE
#include "avr8-stub.h"
#endif

#include "Config.h"
#include "Context.h"
#include "SoilMoistureSensor.h"
#include "Display.h"
#include "Ticker.h"
#include "Utils.h"
#include <algorithm>
#include <utility>
#include <SPI.h>

#include "AT24C.h"

using namespace cz;


#if 1
Context gCtx;

using SoilMoistureSensorTicker = TTicker<SoilMoistureSensor, float>;
SoilMoistureSensorTicker gSoilMoistureSensors[NUM_MOISTURESENSORS] =
{
	{true, gCtx, 0, IO_EXPANDER_VPIN_SENSOR_0, MULTIPLEXER_MOISTURE_SENSOR_0},
	{true, gCtx, 1, IO_EXPANDER_VPIN_SENSOR_1, MULTIPLEXER_MOISTURE_SENSOR_1},
	{true, gCtx, 2, IO_EXPANDER_VPIN_SENSOR_2, MULTIPLEXER_MOISTURE_SENSOR_2},
	#if 1
	{true, gCtx, 3, IO_EXPANDER_VPIN_SENSOR_3, MULTIPLEXER_MOISTURE_SENSOR_3}
	#endif
};

TTicker<Display, float> gDisplay(true, gCtx);

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
#endif

	gCtx.begin();
	gDisplay.getObj().begin();

	for(auto&& ticker : gSoilMoistureSensors)
	{
		ticker.getObj().begin();
	}

	gPreviousTime = millis() / 1000.0f;

	setMotorPins();

	setAllMotorPins(HIGH, LOW);
	delay(1000);
	setAllMotorPins(LOW, LOW);
	delay(3000);

	//gCtx.ioExpander.digitalWrite(IO_EXPANDER_MOTOR_2_INPUT1, HIGH);
	//gCtx.ioExpander.digitalWrite(IO_EXPANDER_MOTOR_2_INPUT2, LOW);
	//delay(3000);
	//gCtx.ioExpander.digitalWrite(IO_EXPANDER_MOTOR_2_INPUT1, LOW);
	//gCtx.ioExpander.digitalWrite(IO_EXPANDER_MOTOR_2_INPUT2, HIGH);
	//delay(3000);
	//gCtx.ioExpander.digitalWrite(IO_EXPANDER_MOTOR_2_INPUT1, LOW);
	//gCtx.ioExpander.digitalWrite(IO_EXPANDER_MOTOR_2_INPUT2, LOW);


}

void loop()
{
	float now = millis() / 1000.0f;
	float deltaSeconds = now - gPreviousTime;
	float countdown = 60*60;

	countdown = std::min(gDisplay.tick(deltaSeconds), countdown);

	for (auto&& ticker : gSoilMoistureSensors)
	{
		countdown = std::min(ticker.tick(deltaSeconds), countdown);
	}

	gPreviousTime = now;
	//CZ_LOG_LN("Waiting %d seconds", (int) countdown);
	delay(countdown*1000);

	gCtx.data.logMoistureSensors();

}

#else

AT24C32 mem(0, Wire);

void setup()
{
#ifdef AVR8_BREAKPOINT_MODE
	debug_init();
#else
	// If using avr-stub, we can't use Serial
	Serial.begin(115200);
#endif

	Wire.begin();

	bool b0 = mem.isPresent();
	CZ_LOG_LN("ATC24_0 : %s", b0 ? "true" : "false");

	cz::runTests(mem);

#if 0
	int t1 = micros();
	CZ_ASSERT(mem.isPresent());
	mem.write8(31, 239);
	int t2 = micros();
	CZ_ASSERT(mem.isPresent());
	int t3 = micros();
	Serial.println(0);

	CZ_LOG_LN("%d, %d(%d), %d(%d)", t1, t2, t2 - t1, t3, t3 - t2);

	mem.write8(1, 253);
	Serial.println(1);

	CZ_ASSERT(mem.hasErrorOccurred() == false);
	mem.write8(4096-1, 254);
	Serial.println(2);

	CZ_ASSERT(mem.hasErrorOccurred() == false);

#endif

}

void loop()
{
}

#endif