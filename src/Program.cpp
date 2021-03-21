#include "Program.h"
#include "Utils.h"
#include <Arduino.h>
#include <Wire.h>
#include <utility/Adafruit_MCP23017.h>

Adafruit_RGBLCDShield lcd;

Program::Program()
{
}

void Program::setup()
{
	Serial.begin(9600);

	// Set number of columns and rows
	lcd.begin(16, 2);
	lcd.clear();
	lcd.print("Hello World!");

	for (uint8_t idx = 0; idx < m_maxSensors; idx++)
	{
		m_sensors[idx].setup(idx, 30 + idx, A12 + idx);
	}
	
	uint8_t relayPins[4] = { 40, 41, 42, 43 };
	m_relayModule.setup(relayPins, m_maxSensors);
}

void Program::tick()
{
	//Serial.print("Millis is: ");
	//Serial.println(millis());

	//unsigned long ms = millis();

	char statusText[100] = "";
	for (uint8_t idx = 0; idx < m_maxSensors; idx++)
	{
		m_sensors[idx].tick();
		uint8_t moisturePercentage = m_sensors[idx].readValue();

		int pumpState = moisturePercentage < 40 ? LOW : HIGH;
		m_relayModule.setInput(idx, pumpState);

		strCatPrintf(statusText, "S%1d %3d%c |",
			idx,
			//m_sensors[idx].getLastPinValue(),
			moisturePercentage,
			pumpState ? 'H' : 'L');

		lcd.setCursor(
			(idx == 0 || idx == 2) ? 0 : 9,
			(idx == 0 || idx == 1) ? 0 : 1);

		lcd.print(statusText);
		CZ_LOG_LN(statusText);

		statusText[0] = 0;
	}

	Serial.println(millis());
	delay(250);
}


#if 0
/*********************

Example code for the Adafruit RGB Character LCD Shield and Library

This code displays text on the shield, and also reads the buttons on the keypad.
When a button is pressed, the backlight changes color.

**********************/

// include the library code:
#include <Wire.h>
#include <Adafruit_RGBLCDShield.h>
#include <utility/Adafruit_MCP23017.h>


// The shield uses the I2C SCL and SDA pins. On classic Arduinos
// this is Analog 4 and 5 so you can't use those for analogRead() anymore
// However, you can connect other I2C sensors to the I2C bus and share
// the I2C bus.
Adafruit_RGBLCDShield lcd = Adafruit_RGBLCDShield();

// These #defines make it easy to set the backlight color
#define RED 0x1
#define YELLOW 0x3
#define GREEN 0x2
#define TEAL 0x6
#define BLUE 0x4
#define VIOLET 0x5
#define WHITE 0x7

void setup() {
  // Debugging output
  Serial.begin(9600);
  // set up the LCD's number of columns and rows: 
  lcd.begin(16, 2);

  // Print a message to the LCD. We track how long it takes since
  // this library has been optimized a bit and we're proud of it :)
  int time = millis();
  lcd.print("Hello, world!");
  time = millis() - time;
  Serial.print("Took "); Serial.print(time); Serial.println(" ms");
  lcd.setBacklight(WHITE);
}

uint8_t i=0;
void loop() {
  // set the cursor to column 0, line 1
  // (note: line 1 is the second row, since counting begins with 0):
  lcd.setCursor(0, 1);
  // print the number of seconds since reset:
  lcd.print(millis()/1000);

  uint8_t buttons = lcd.readButtons();

  if (buttons) {
    lcd.clear();
    lcd.setCursor(0,0);
    if (buttons & BUTTON_UP) {
      lcd.print("UP ");
      lcd.setBacklight(RED);
    }
    if (buttons & BUTTON_DOWN) {
      lcd.print("DOWN ");
      lcd.setBacklight(YELLOW);
    }
    if (buttons & BUTTON_LEFT) {
      lcd.print("LEFT ");
      lcd.setBacklight(GREEN);
    }
    if (buttons & BUTTON_RIGHT) {
      lcd.print("RIGHT ");
      lcd.setBacklight(TEAL);
    }
    if (buttons & BUTTON_SELECT) {
      lcd.print("SELECT ");
      lcd.setBacklight(VIOLET);
      //lcd.setBacklight(0);
      //lcd.noDisplay();
    }
  }
}

#endif
