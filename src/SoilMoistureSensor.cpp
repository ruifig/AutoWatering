#include "SoilMoistureSensor.h"
#include "Utils.h"
#include <Arduino.h>

/*
How to fix or improve sensor response:
	https://forum.arduino.cc/index.php?topic=597585.0
	https://www.youtube.com/watch?v=QGCrtXf8YSs
*/

SoilMoistureSensor::SoilMoistureSensor()
{
}

void SoilMoistureSensor::setup(uint8_t id, uint8_t vinPin, uint8_t dataPin)
{
	m_id = id;
	m_vinPin = vinPin;
	m_dataPin = dataPin;

	pinMode(m_vinPin, OUTPUT);
	pinMode(m_dataPin, INPUT);

	// Switch the sensor off
	digitalWrite(m_vinPin, LOW);
}

void SoilMoistureSensor::tick()
{
	/*
	int pinValue = analogRead(m_pin);
	Serial.print("Sensor value (pin ");
	Serial.print(m_pin);
	Serial.print(") = ");
	Serial.println(pinValue);
	*/
}

uint8_t SoilMoistureSensor::readValue()
{
	//
	// Take a measurement by turning the sensor ON, do a ready, then switch it OFF
	//
	digitalWrite(m_vinPin, HIGH); // Switch the sensor ON
	delay(200); // Delay to give the sensor time to read the moisture level
	m_lastPinValue = analogRead(m_dataPin);

	if (m_lastPinValue > m_airValue)
	{
		m_airValue = m_lastPinValue;
	}
	else if (m_lastPinValue < m_waterValue)
	{
		m_waterValue = m_lastPinValue;
	}
	digitalWrite(m_vinPin, LOW); // Switch the sensor OFF

	int percentage = map(m_lastPinValue, m_airValue, m_waterValue, 0, 100);

	return percentage;
}

void RelayModule::setup(const uint8_t* pinIn, int numChannels)
{
	if (numChannels > m_maxChannels)
	{
		assert(false);
		return;
	}

	m_numChannels = numChannels;
	memcpy(m_pinIns, pinIn, numChannels);

	for (int idx = 0; idx < m_numChannels; idx++)
	{
		pinMode(m_pinIns[idx], OUTPUT);
		digitalWrite(m_pinIns[idx], LOW);
	}
}

void RelayModule::setInput(uint8_t idx, uint8_t state)
{
	if (idx < m_numChannels)
	{
		CZ_LOG_LN("Signalling relay channel %d (pin %d) to %d", idx, m_pinIns[idx], (int)state);
		digitalWrite(m_pinIns[idx], state);
		//analogWrite(m_pinIns[idx], 255);
	}
}

#if 0
void RelayModule::signalHigh(uint8_t idx)
{
	if (idx < m_numChannels)
	{
		CZ_LOG_LN("Signalling relay channel %d (pin %d)", idx, m_pinIns[idx]);
		digitalWrite(m_pinIns[idx], HIGH);
	}
}

void RelayModule::signalLow(uint8_t idx)
{
	if (idx < m_numChannels)
	{
		CZ_LOG_LN("Signalling relay channel %d (pin %d)", idx, m_pinIns[idx]);
		digitalWrite(m_pinIns[idx], LOW);
	}
}

void RelayModule::signal(uint8_t idx, unsigned long duration)
{
	if (idx < m_numChannels)
	{
		CZ_LOG_LN("Signalling relay channel %d (pin %d)", idx, m_pinIns[idx]);
		digitalWrite(m_pinIns[idx], HIGH);
		delay(duration);
		digitalWrite(m_pinIns[idx], LOW);
	}
}
#endif
