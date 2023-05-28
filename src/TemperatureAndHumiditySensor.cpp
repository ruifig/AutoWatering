#include "TemperatureAndHumiditySensor.h"
#include "Context.h"
#include "crazygaze/micromuc/Logging.h"
#include "crazygaze/micromuc/Profiler.h"
#include "crazygaze/micromuc/MathUtils.h"
#include "crazygaze/micromuc/StringUtils.h"
#include <Arduino.h>
#include <algorithm>

namespace cz
{

const char* const TemperatureAndHumiditySensor::ms_stateNames[3] =
{
	"Initializing",
	"Idle",
	"Reading"
};

TemperatureAndHumiditySensor::TemperatureAndHumiditySensor()
{
	// We only start ticking once we receive a ConfigReady event
	stopTicking();
}

bool TemperatureAndHumiditySensor::initImpl()
{
	CZ_LOG(logDefault, Log, "Initializing the temperature/humidity sensor");
	onEnterState();
	return true;
}

float TemperatureAndHumiditySensor::tick(float deltaSeconds)
{
	PROFILE_SCOPE(F("TemperatureAndHumiditySensor"));

	m_timeInState += deltaSeconds;
	m_timeSinceLastRead += deltaSeconds;

	m_nextTickWait = 1.0f;

	switch (m_state)
	{
	case State::Initializing:
		changeToState(State::Idle);
		break;

	case State::Idle:
		if (m_timeSinceLastRead >= AW_THSENSOR_SAMPLINGINTERVAL)
		{
			changeToState(State::Reading);
		}
		break;

	case State::Reading:
		{
			CZ_LOG(logDefault, Verbose, F("TemperatureAndHumiditySensor: Starting read"))
			float temperature, humidity;
			readSensor(temperature, humidity);
			gCtx.data.setTemperatureReading(temperature);
			gCtx.data.setHumidityReading(humidity);

			if (isnan(temperature))
			{
				CZ_LOG(logDefault, Verbose, F("TemperatureAndHumiditySensor: Error reading temperature"));
				resetSensor();
			}
			else if (isnan(humidity))
			{
				CZ_LOG(logDefault, Verbose, F("TemperatureAndHumiditySensor: Error reading humidity"));
				resetSensor();
			}

			m_timeSinceLastRead = 0;
			changeToState(State::Idle);
		}
		break;

	default:
		CZ_UNEXPECTED();
	}

	return m_nextTickWait;
}

void TemperatureAndHumiditySensor::readSensor(float& temperature, float& humidity)
{
#if AW_MOCK_COMPONENTS
	static int counter = 0;
	counter = (counter+1) % 10;
	temperature = 20.0f + counter / 10.0f;
	humidity = 50.0f + counter / 10.0f;
#else
	temperature = m_htu.readTemperature();
	humidity = m_htu.readHumidity();
#endif
}

void TemperatureAndHumiditySensor::changeToState(State newState)
{
	CZ_LOG(logDefault, Verbose, F("TemperatureAndHumiditySensor::%s: %ssec %s->%s")
		, __FUNCTION__
		, *FloatToString(m_timeInState)
		, ms_stateNames[(int)m_state]
		, ms_stateNames[(int)newState]);

	onLeaveState();
	m_state = newState;
	m_timeInState = 0.0f;
	onEnterState();
}

void TemperatureAndHumiditySensor::resetSensor()
{
	m_htu.reset();
}

void TemperatureAndHumiditySensor::onLeaveState()
{
	switch(m_state)
	{
	case State::Initializing:
		break;

	case State::Idle:
		break;

	case State::Reading:
		break;
	}

}

void TemperatureAndHumiditySensor::onEnterState()
{
	switch (m_state)
	{
	case State::Initializing:
		if (!m_htu.begin())
		{
			CZ_LOG(logDefault, Error, F("Error initializing temperature/humidity sensor"));
		}
		break;

	case State::Idle:
		break;

	case State::Reading:
		break;

	default:
		CZ_UNEXPECTED();
	}
}

void TemperatureAndHumiditySensor::onEvent(const Event& evt)
{
	switch(evt.type)
	{
		case Event::ConfigReady:
			startTicking();
		break;
	}
}

#if AW_THSENSOR_ENABLED
	TemperatureAndHumiditySensor gTempAndHumiditySensor;
#endif

} // namespace cz
