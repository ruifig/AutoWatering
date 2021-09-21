#include "SoilMoistureSensor.h"
#include "Utils.h"
#include <crazygaze/micromuc/Logging.h>
#include <crazygaze/micromuc/Profiler.h>
#include <crazygaze/micromuc/MathUtils.h>
#include <Arduino.h>

namespace cz
{

#if CZ_LOG_ENABLED
const char* const SoilMoistureSensor::ms_stateNames[4] =
{
	"Initializing",
	"PoweredDown",
	"PoweringUp",
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
	PROFILE_SCOPE(F("MoistureSensor"));

	m_timeInState += deltaSeconds;

#if FASTER_ITERATION
	m_nextTickWait = 0.001f;
#else
	m_nextTickWait = 0.2f;
#endif

	switch (m_state)
	{
	case State::Initializing:
		// From Initializing, we jump straight to a first reading
		changeToState(State::PoweringUp);
		break;

	case State::PoweredDown:
		if (m_timeInState >= m_ctx.data.getGroupData(m_index).getSamplingInterval())
		{
			changeToState(State::PoweringUp);
		}
		break;

	case State::PoweringUp:
		if (m_timeInState >= MOISTURESENSOR_POWERUP_WAIT)
		{
			tryEnterReadingState();
		}
		break;

	case State::Reading:
		{
			GroupData& data = m_ctx.data.getGroupData(m_index);

			// Using a function to read the sensor, so we can provide a mock value when using the mock version
			int currentValue = readSensor();
			data.setMoistureSensorValues(currentValue);
			#if FASTER_ITERATION && 0
				for(int i=0; i<5; i++)
				{
					data.setMoistureSensorValues(currentValue, airValue, waterValue);
				}
			#endif
			changeToState(State::PoweredDown);
		}
		break;

	default:
		CZ_UNEXPECTED();
	}

	return m_nextTickWait;
}

int SoilMoistureSensor::readSensor()
{
	int currentValue = m_ctx.mux.read(m_dataPin);
	CZ_LOG(logDefault, Log, F("SoilMoistureSensor(%d) : %d"), m_index, currentValue);
	return currentValue;
}

void SoilMoistureSensor::onEvent(const Event& evt)
{
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

	case State::PoweringUp:
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

	case State::PoweringUp:
		//  To take a measurement, we turn the sensor ON, wait a bit, then switch it off
		m_ctx.ioExpander.digitalWrite(m_vinPin, HIGH);
		m_nextTickWait = MOISTURESENSOR_POWERUP_WAIT;
		break;

	case State::Reading:
		break;

	default:
		CZ_UNEXPECTED();
	}
}

//////////////////////////////////////////////////////////////////////////
// MockSoilMoistureSensor
//////////////////////////////////////////////////////////////////////////

void MockSoilMoistureSensor::begin()
{
	SoilMoistureSensor::begin();

	m_mock.dryValue = random(540,590);
	m_mock.waterValue = random(160, 210);
	m_mock.targetValue = m_mock.currentValue = m_mock.dryValue;
}

float MockSoilMoistureSensor::tick(float deltaSeconds)
{
	float tickResult = SoilMoistureSensor::tick(deltaSeconds);

	if (m_mock.motorIsOn)
	{
		m_mock.targetValue -= m_mock.targetValueOnRate * deltaSeconds;
		m_mock.targetValue = max(m_mock.waterValue, m_mock.targetValue);
	}
	else
	{
		m_mock.targetValue += m_mock.targetValueOffRate * deltaSeconds;
		m_mock.targetValue = min(m_mock.dryValue, m_mock.targetValue);
	}

	if (isNearlyEqual(m_mock.currentValue, m_mock.targetValue))
	{
		// Do nothing
	}
	else if (m_mock.currentValue < m_mock.targetValue)
	{
		m_mock.currentValue += m_mock.currentValueChaseRate * deltaSeconds;
		m_mock.currentValue = min(m_mock.currentValue, m_mock.targetValue);
	}
	else // currentValue > targetValue
	{
		m_mock.currentValue -= m_mock.currentValueChaseRate * deltaSeconds;
		m_mock.currentValue = max(m_mock.currentValue, m_mock.targetValue);
	}

	return tickResult;
} 

void MockSoilMoistureSensor::onEvent(const Event& evt)
{
	SoilMoistureSensor::onEvent(evt);

	if (evt.type == Event::Motor)
	{
		const MotorEvent& e = static_cast<const MotorEvent&>(evt);
		if (e.index == m_index)
		{
			m_mock.motorIsOn = e.started;
		}
	}

}

int MockSoilMoistureSensor::readSensor()
{
	char buf1[10];
	char buf2[10];

	CZ_LOG(logDefault, Log, F("MockSoilMoistureSensor(%d) : %s, target=%s")
		, m_index
		, dtostrf(m_mock.currentValue, 0, 3, buf1)
		, dtostrf(m_mock.targetValue , 0, 3, buf2))

	return static_cast<int>(m_mock.currentValue);
}

}  // namespace cz
