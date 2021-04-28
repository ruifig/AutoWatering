#include "SoilMoistureSensor.h"
#include "Utils.h"
#include <crazygaze/micromuc/Logging.h>
#include <Arduino.h>

namespace cz
{

#if CZ_LOG_ENABLED
const char* const SoilMoistureSensor::ms_stateNames[3] =
{
	"Initializing",
	"PoweredDown",
	"Reading"
};
#endif

SoilMoistureSensor::SoilMoistureSensor(Context& ctx, uint8_t index, IOExpanderPin vinPin, MultiplexerPin dataPin)
	: m_ctx(ctx)
	, m_index(index)
	, m_vinPin(vinPin)
	, m_dataPin(dataPin)
{
}

void SoilMoistureSensor::begin()
{
	onEnterState();
}

bool SoilMoistureSensor::tryEnterReadingState()
{
	// From Initializing, we jump straight to a first reading
	if (m_ctx.data.tryAcquireMoistureSensorMutex())
	{
		changeToState(State::Reading);
		return true;
	}
	else
	{
		return false;
	}
}

float SoilMoistureSensor::tick(float deltaSeconds)
{
	m_timeInState += deltaSeconds;
	m_nextTickWait = 0.2f;

	switch (m_state)
	{
	case State::Initializing:
		// From Initializing, we jump straight to a first reading
		tryEnterReadingState();
		break;

	case State::PoweredDown:
		if (m_timeInState >= m_ctx.data.getGroupData(m_index).getSamplingInterval())
		{
			tryEnterReadingState();
		}
		break;

	case State::Reading:

		if (m_timeInState >= MOISTURESENSOR_POWERUP_WAIT)
		{
			GroupData& data = m_ctx.data.getGroupData(m_index);
			int airValue = data.getAirValue();
			int waterValue = data.getWaterValue();

			// Read the sensor
			int currentValue = m_ctx.mux.read(m_dataPin);
			if (currentValue > airValue)
			{
				airValue = currentValue;
			}
			else if (currentValue < waterValue)
			{
				waterValue = currentValue;
			}

			//CZ_LOG(logDefault, Log, F("SoilMoistureSensor(%d) : %d"), m_index, currentValue);
			data.setMoistureSensorValues(currentValue, airValue, waterValue);
			changeToState(State::PoweredDown);
		}
		break;

	default:
		CZ_UNEXPECTED();
	}

	return m_nextTickWait;
}

void SoilMoistureSensor::changeToState(State newState)
{
	#if 0
	CZ_LOG(logDefault, Log, F("SoilMoistureSensor(%d)::%s: %dms %s->%s")
		, (int)m_index
		, __FUNCTION__
		, (int)(m_timeInState * 1000.f)
		, ms_stateNames[(int)m_state]
		, ms_stateNames[(int)newState]);
	#endif

	onLeaveState();
	m_state = newState;
	m_timeInState = 0.0f;
	onEnterState();
}

void SoilMoistureSensor::onLeaveState()
{
	switch (m_state)
	{
	case State::Initializing:
		break;

	case State::PoweredDown:
		break;

	case State::Reading:
		//  Turn power off
		m_ctx.ioExpander.digitalWrite(m_vinPin, LOW);
		// release the mutex so other sensors can read
		m_ctx.data.releaseMoistureSensorMutex();
		break;

	default:
		CZ_UNEXPECTED();
	}
}

void SoilMoistureSensor::onEnterState()
{
	switch (m_state)
	{
	case State::Initializing:
		m_ctx.ioExpander.pinMode(m_vinPin, OUTPUT);
		// Switch the sensor off
		m_ctx.ioExpander.digitalWrite(m_vinPin, LOW);
		break;

	case State::PoweredDown:
		break;

	case State::Reading:
		//  To take a measurement, we turn the sensor ON, wait a bit, the switch it off
		m_ctx.ioExpander.digitalWrite(m_vinPin, HIGH);
		m_nextTickWait = MOISTURESENSOR_POWERUP_WAIT;
		break;

	default:
		CZ_UNEXPECTED();
	}
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
