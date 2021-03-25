#include "SoilMoistureSensor.h"
#include "Utils.h"
#include <Arduino.h>

namespace cz
{

SoilMoistureSensor::SoilMoistureSensor(Context& ctx, uint8_t index, IOExpanderPin vinPin, MultiplexerPin dataPin)
	: m_ctx(ctx)
	, m_index(index)
	, m_vinPin(vinPin)
	, m_dataPin(dataPin)
{
}

void SoilMoistureSensor::setup()
{
	m_ctx.ioExpander.pinMode(m_vinPin, OUTPUT);
	// Switch the sensor off
	m_ctx.ioExpander.digitalWrite(m_vinPin, LOW);
}

float SoilMoistureSensor::tick(float deltaSeconds)
{
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

	return m_ctx.data.getSoilMoistureSensor(m_index).samplingIntervalSeconds;
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
