#include <Arduino.h>

#include "Config.h"
#include "ProgramData.h"
#include "SoilMoistureSensor.h"
#include "Display.h"
#include "Ticker.h"
#include "Utils.h"
#include <algorithm>

using namespace cz;

ProgramData gData;
using SoilMoistureSensorTicker = TTicker<SoilMoistureSensor, float>;
SoilMoistureSensorTicker gSoilMoistureSensors[NUM_SENSORS];
TTicker<Display, float> gDisplay;
Adafruit_RGBLCDShield lcd;

float gPreviousTime = 0;
void setup()
{
	Serial.begin(9600);

	gDisplay.getObj().setup(lcd, gData);

	for (int idx = 0; idx < NUM_SENSORS; idx++)
	{
		gSoilMoistureSensors[idx].getObj().setup(gData, idx, 30 + idx, PIN_A12 + idx);
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
