#include "TemperatureAndHumiditySensor.h"
#include "Context.h"
#include "crazygaze/micromuc/Logging.h"
#include "crazygaze/micromuc/Profiler.h"
#include "crazygaze/micromuc/MathUtils.h"
#include "crazygaze/micromuc/StringUtils.h"
#include <Arduino.h>
#include <algorithm>
#include <DHT.h>

namespace cz
{

#define DHT22_POWERUP_TIME 2.0f

#if CZ_LOG_ENABLED
const char* const TemperatureAndHumiditySensor::ms_stateNames[3] =
{
	"Initializing",
	"PoweredDown",
	"Reading"
};
#endif

TemperatureAndHumiditySensor::TemperatureAndHumiditySensor(IOExpanderPin vinPin, MultiplexerPin dataPin)
	: m_vinPin(vinPin)
	, m_dataPin(dataPin)
{
}

void TemperatureAndHumiditySensor::begin()
{
	onEnterState();
}

bool TemperatureAndHumiditySensor::tryEnterReadingState()
{
	if (gCtx.data.tryAcquireMuxMutex())
	{
		changeToState(State::Reading);
		return true;
	}
	else
	{
		return false;
	}
}

float TemperatureAndHumiditySensor::tick(float deltaSeconds)
{
	PROFILE_SCOPE(F("TemperatureAndHumiditySensor"));

	m_timeInState += deltaSeconds;
	m_timeSinceLastRead += deltaSeconds;

	m_nextTickWait = 0.5f;

	switch (m_state)
	{
	case State::Initializing:
		changeToState(State::PoweredDown);
		break;

	case State::PoweredDown:
		if (m_timeSinceLastRead >= TEMPSENSOR_DEFAULT_SAMPLINGINTERVAL)
		{
			tryEnterReadingState();
		}
		break;

	case State::Reading:
		if (m_timeInState >= DHT22_POWERUP_TIME)
		{
			CZ_LOG(logDefault, Verbose, F("TemperatureAndHumiditySensor: Power up finished. Starting read"))
			float temperature, humidity;
			readSensor(temperature, humidity);
			gCtx.data.setTemperatureReading(temperature);
			gCtx.data.setHumidityReading(humidity);

			// #TODO : If reading failed (e.g: returned NAN), then we need to read again instead of just resetting the timer
			m_timeSinceLastRead = 0;
			changeToState(State::PoweredDown);
		}
		break;

	default:
		CZ_UNEXPECTED();
	}

	return m_nextTickWait;
}

void TemperatureAndHumiditySensor::readSensor(float& temperature, float& humidity)
{
	//CZ_LOG(logDefault, Log, F("TemperatureAndHumiditySensor: 1"));
	gCtx.mux.setChannel(m_dataPin);
	//CZ_LOG(logDefault, Log, F("TemperatureAndHumiditySensor: 2"));
	DHT dht(gCtx.mux.getMCUZPin().raw, DHT22);
	//CZ_LOG(logDefault, Log, F("TemperatureAndHumiditySensor: 3"));
	dht.begin();

	//CZ_LOG(logDefault, Log, F("TemperatureAndHumiditySensor: 4"));
	humidity = dht.readHumidity();
	//CZ_LOG(logDefault, Log, F("TemperatureAndHumiditySensor: 5"));
	temperature = dht.readTemperature();

	//
	// Clean up mux usage
	pinMode(gCtx.mux.getMCUZPin(), OUTPUT);
	digitalWrite(gCtx.mux.getMCUZPin(), LOW);
	pinMode(gCtx.mux.getMCUZPin(), INPUT);

	//CZ_LOG(logDefault, Log, F("TemperatureAndHumiditySensor: Finished reading DHT22 sensor"));
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

void TemperatureAndHumiditySensor::onLeaveState()
{
	switch(m_state)
	{
	case State::Initializing:
		break;

	case State::PoweredDown:
		break;

	case State::Reading:
		//  Turn power off
		gCtx.ioExpander.digitalWrite(m_vinPin, LOW);
		// release the mutex so other sensors can read
		gCtx.data.releaseMuxMutex();
		break;
	}

}

void TemperatureAndHumiditySensor::onEnterState()
{
	switch (m_state)
	{
	
	case State::Initializing:
		gCtx.ioExpander.pinMode(m_vinPin, OUTPUT);
		// Switch the sensor off
		gCtx.ioExpander.digitalWrite(m_vinPin, LOW);
		break;

	case State::PoweredDown:
		break;

	case State::Reading:
		// To take a measurement, we turn the sensor ON, wait a bit, then switch it off
		gCtx.ioExpander.digitalWrite(m_vinPin, HIGH);
		m_nextTickWait = DHT22_POWERUP_TIME;
		break;

	default:
		CZ_UNEXPECTED();
	}
}

void TemperatureAndHumiditySensor::onEvent(const Event& evt)
{
}

} // namespace cz
