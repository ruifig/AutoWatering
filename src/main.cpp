#include <Arduino.h>

#include "Config.h"
#include "Context.h"
#include "SoilMoistureSensor.h"
#include "Display.h"
#include "Ticker.h"
#include "Utils.h"
#include <algorithm>
#include <utility>
#include "MCP23S17Wrapper.h"
#include <SPI.h>
#include "Mux16Channels.h"

using namespace cz;

Context gCtx;

using SoilMoistureSensorTicker = TTicker<SoilMoistureSensor, float>;
SoilMoistureSensorTicker gSoilMoistureSensors[4] =
{
	{true, gCtx, 0, IO_EXPANDER_VPIN_SENSOR_0, MULTIPLEXER_MOISTURE_SENSOR_0},
	{true, gCtx, 1, IO_EXPANDER_VPIN_SENSOR_1, MULTIPLEXER_MOISTURE_SENSOR_1},
	{true, gCtx, 2, IO_EXPANDER_VPIN_SENSOR_2, MULTIPLEXER_MOISTURE_SENSOR_2},
	{true, gCtx, 3, IO_EXPANDER_VPIN_SENSOR_3, MULTIPLEXER_MOISTURE_SENSOR_3}
};

TTicker<Display, float> gDisplay;
Adafruit_RGBLCDShield lcd;

float gPreviousTime = 0;
void setup()
{
	Serial.begin(9600);

	gDisplay.getObj().setup(gCtx, lcd);

	pinMode(ARDUINO_MULTIPLEXER_ZPIN.raw, INPUT);

	for(auto&& ticker : gSoilMoistureSensors)
	{
		ticker.getObj().setup();
	}

	gPreviousTime = millis() / 1000.0f;
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

}
