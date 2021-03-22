#include "SoilMoistureSensor.h"

#include <Arduino.h>

#include "ProgramData.h"
#include "Utils.h"

namespace cz
{
SoilMoistureSensor::SoilMoistureSensor(ProgramData& data, uint8_t index, uint8_t vinPin, uint8_t dataPin)
{
	setup(data, index, vinPin, dataPin);
}

void SoilMoistureSensor::setup(ProgramData& data, uint8_t index, uint8_t vinPin, uint8_t dataPin)
{
	m_data = &data;
	m_index = index;
	m_vinPin = vinPin;
	m_dataPin = dataPin;

	pinMode(m_vinPin, OUTPUT);
	pinMode(m_dataPin, INPUT);
	// Switch the sensor off
	digitalWrite(m_vinPin, LOW);
}

float SoilMoistureSensor::tick(float deltaSeconds)
{
	CZ_ASSERT(m_data);

	switch (m_state)
	{
	case State::Off:
		break;

	case State::PoweringUp:
		break;

	case State::Reading:
		// TODO : Fill me
		break;

	default:
		CZ_UNEXPECTED();
	}

	return m_data->getSoilMoistureSensor(m_index).samplingIntervalSeconds;
}

#if 0
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

#endif

}  // namespace cz