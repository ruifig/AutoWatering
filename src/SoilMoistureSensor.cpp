#include "SoilMoistureSensor.h"
#include "Context.h"
#include "crazygaze/micromuc/Logging.h"
#include "crazygaze/micromuc/Profiler.h"
#include "crazygaze/micromuc/MathUtils.h"
#include "crazygaze/micromuc/StringUtils.h"
#include <Arduino.h>
#include <algorithm>
#include <crazygaze/micromuc/StringUtils.h>

namespace cz
{

const char* const RealSoilMoistureSensor::ms_stateNames[3] =
{
	"Initializing",
	"PoweredDown",
	"QueuedForReading"
	"Reading"
};

RealSoilMoistureSensor::SemaphoreQueue RealSoilMoistureSensor::ms_semaphoreQueue;

RealSoilMoistureSensor::RealSoilMoistureSensor(uint8_t index, DigitalOutputPin& vinPin, AnalogInputPin& dataPin)
	: m_index(index)
	, m_vinPin(vinPin)
	, m_dataPin(dataPin)
	, m_queueHandle(ms_semaphoreQueue.createHandle())
{
}

const char* RealSoilMoistureSensor::getName() const
{
	return formatString("SoilMoistureSensor%d", m_index);
}

bool RealSoilMoistureSensor::initImpl()
{
	onEnterState();
	return true;
}

void RealSoilMoistureSensor::tryEnterReadingState()
{
	if (m_queueHandle.tryAcquire(true))
	{
		changeToState(State::Reading);
	}
	else if (m_state != State::QueuedForReading)
	{
		changeToState(State::QueuedForReading);
	}
}

float RealSoilMoistureSensor::tick(float deltaSeconds)
{
	PROFILE_SCOPE(F("MoistureSensor"));

	m_timeInState += deltaSeconds;
	m_timeSinceLastRead += deltaSeconds;
	GroupData& data = gCtx.data.getGroupData(m_index);
	bool isSelectedGroup = gCtx.data.getSelectedGroup() == &data ? true : false;

#if AW_FASTER_ITERATION
	m_nextTickWait = 0.05f;
#else
	m_nextTickWait = 0.2f;
#endif

	switch (m_state)
	{
	case State::Initializing:
		changeToState(State::PoweredDown);
		break;

	case State::PoweredDown:
		if (data.isRunning() || data.isInConfigMenu())
		{
			float samplingInterval = data.isInConfigMenu() ? AW_MOISTURESENSOR_CALIBRATION_SAMPLINGINTERVAL : data.getSamplingInterval();
			if (m_timeSinceLastRead >= samplingInterval)
			{
				tryEnterReadingState();
			}
		}
		break;

	case State::QueuedForReading:
		{
			tryEnterReadingState();
		}
		break;

	case State::Reading:
		{
			if (m_timeInState >= AW_MOISTURESENSOR_POWERUP_WAIT)
			{
				// Using a function to read the sensor, so we can provide a mock value when using the mock version
				SensorReading sample = readSensor();
				data.setMoistureSensorValues(sample);
				m_timeSinceLastRead = 0;
				changeToState(State::PoweredDown);
			}
		}
	break;

	default:
		CZ_UNEXPECTED();
	}

	return m_nextTickWait;
}

SensorReading RealSoilMoistureSensor::readSensor()
{
	unsigned long startMicros = micros();
	constexpr int numSamples = 30;
	int samples[numSamples];

	for(auto&& s : samples)
	{
		s = m_dataPin.read();
	}

	StandardDeviation res = calcStandardDeviation(samples, numSamples);
	SensorReading sample(static_cast<unsigned int>(res.mean), res.stdDeviation);

	unsigned int long endMicros = micros() - startMicros;
	CZ_LOG(logDefault, Log, F("SoilMoistureSensor(%d) : duration=%4.2fms Mean=%u, stdDeviation=%4.2f")
		, m_index
		, ((float)endMicros/1000.0f)
		, sample.meanValue
		, sample.standardDeviation);

#if 0
	char buf[2048];
	buf[0] = 0;
	for(auto&& s : samples)
	{
		strCatPrintf(buf, "%d,", s);
	}
	CZ_LOG(logDefault, Log, F("Samples[%d]={%s}"), numSamples, buf);
#endif

	return sample;
}

void RealSoilMoistureSensor::onEvent(const Event& evt)
{
	switch(evt.type)
	{
		case Event::ConfigLoad:
		case Event::GroupOnOff:
			if (!gCtx.data.getGroupData(m_index).isRunning())
			{
				changeToState(State::PoweredDown);
			}
			break;
		break;
	}
}

void RealSoilMoistureSensor::changeToState(State newState)
{
	CZ_LOG(logDefault, Verbose, F("SoilMoistureSensor(%d)::%s: %ssec %s->%s")
		, (int)m_index
		, __FUNCTION__
		, *FloatToString(m_timeInState)
		, ms_stateNames[(int)m_state]
		, ms_stateNames[(int)newState]);

	onLeaveState();
	m_state = newState;
	m_timeInState = 0.0f;
	onEnterState();
}

void RealSoilMoistureSensor::onLeaveState()
{
	switch (m_state)
	{
	case State::Initializing:
		break;

	case State::PoweredDown:
		break;

	case State::QueuedForReading:
		break;

	case State::Reading:
		//  Turn power off
		m_vinPin.write(PinStatus::LOW);
		// Done using the pin, so disable it. This is necessary so muxed pins work when a single MCU analog pin is muxed with multiple muxes
		m_dataPin.disable();
		break;

	default:
		CZ_UNEXPECTED();
	}
}

void RealSoilMoistureSensor::onEnterState()
{
	switch (m_state)
	{
	case State::Initializing:
		// Switch the sensor off
		m_vinPin.write(PinStatus::LOW);
		break;

	case State::PoweredDown:
		// Release the semaphore queue handle, so other sensors can get their turn
		m_queueHandle.release();
		break;

	case State::QueuedForReading:
		break;

	case State::Reading:
		// Enable the pin before reading This is necessary so muxed pins work when a single MCU analog pin is muxed with multiple muxes
		m_dataPin.enable();
		//  To take a measurement, we turn the sensor ON, wait a bit, then switch it off
		m_vinPin.write(PinStatus::HIGH);
		m_nextTickWait = AW_MOISTURESENSOR_POWERUP_WAIT;
		break;

	default:
		CZ_UNEXPECTED();
	}
}

//////////////////////////////////////////////////////////////////////////
// MockSoilMoistureSensor
//////////////////////////////////////////////////////////////////////////

bool MockSoilMoistureSensor::initImpl()
{
	if (!RealSoilMoistureSensor::initImpl())
	{
		return false;
	}

	m_mock.dryValue = random(540,590);
	m_mock.waterValue = random(160, 210);
	m_mock.targetValue = m_mock.currentValue = random(m_mock.waterValue, m_mock.dryValue);
	return true;
}

float MockSoilMoistureSensor::tick(float deltaSeconds)
{
	float tickResult = RealSoilMoistureSensor::tick(deltaSeconds);

	// Note: The target value needs to be updated before the current value, so that once the motor is off and things stabilize
	// the current value and target value will match at the end fo the tick
	if (m_mock.motorIsOn)
	{
		m_mock.targetValue -= m_mock.targetValueOnRate * deltaSeconds;
		m_mock.targetValue = std::max((float)m_mock.waterValue, m_mock.targetValue);
	}
	else
	{
		m_mock.targetValue += m_mock.targetValueOffRate * deltaSeconds;
		m_mock.targetValue = std::min((float)m_mock.dryValue, m_mock.targetValue);
	}

	if (isNearlyEqual(m_mock.currentValue, m_mock.targetValue))
	{
		// Do nothing
	}
	else if (m_mock.currentValueChaseDelay > 0)
	{
		// If we have an active delay, then update the delay and do nothing else
		m_mock.currentValueChaseDelay -= deltaSeconds;
	}
	else if (m_mock.currentValue < m_mock.targetValue)
	{
		m_mock.currentValue += m_mock.currentValueChaseRate * deltaSeconds;
		m_mock.currentValue = std::min(m_mock.currentValue, m_mock.targetValue);
	}
	else // currentValue > targetValue
	{
		m_mock.currentValue -= m_mock.currentValueChaseRate * deltaSeconds;
		m_mock.currentValue = std::max(m_mock.currentValue, m_mock.targetValue);
	}

	return tickResult;
} 

void MockSoilMoistureSensor::onEvent(const Event& evt)
{
	RealSoilMoistureSensor::onEvent(evt);

	if (evt.type == Event::Motor)
	{
		const MotorEvent& e = static_cast<const MotorEvent&>(evt);
		if (e.index == m_index)
		{
			m_mock.motorIsOn = e.started;

			// If things are stable (current value caught up with target value), and there is no currently active delay,
			// then start a delay
			if (isNearlyEqual(m_mock.currentValue, m_mock.targetValue) && m_mock.currentValueChaseDelay<=0)
			{
				m_mock.currentValueChaseDelay = ms_delayChaseValue;
			}
		}
	}
	else if (evt.type == Event::SetMockSensorValue)
	{
		const SetMockSensorValueEvent& e = static_cast<const SetMockSensorValueEvent&>(evt);
		if (e.index == m_index)
		{
			m_mock.targetValue = m_mock.currentValue = std::clamp(e.value, m_mock.waterValue, m_mock.dryValue);
			m_mock.currentValueChaseDelay = 0;
		}
	}
	else if (evt.type == Event::SetMockSensorErrorStatus)
	{
		const SetMockSensorErrorStatusEvent& e = static_cast<const SetMockSensorErrorStatusEvent&>(evt);
		if (e.index == m_index)
		{
			m_mock.status = e.status;
		}
	}
}

SensorReading MockSoilMoistureSensor::readSensor()
{
	if (m_mock.status == SensorReading::Status::Valid)
	{
		CZ_LOG(logDefault, Verbose, F("MockSoilMoistureSensor(%d) : %s, target=%s")
			, m_index
			, *FloatToString(m_mock.currentValue)
			, *FloatToString(m_mock.targetValue))

		return SensorReading(m_mock.currentValue, 3.0f);
	}
	else
	{
		SensorReading res;
		if (m_mock.status == SensorReading::Status::NoSensor)
		{
			res = SensorReading(random(260, 530), AW_MOISTURESENSOR_ACCEPTABLE_STANDARD_DEVIATION + 1);
		}
		else
		{
			res = SensorReading(random(10,AW_MOISTURESENSOR_ACCEPTABLE_MIN_VALUE-1), 5);
		}

		CZ_LOG(logDefault, Verbose, F("MockSoilMoistureSensor(%d) : ERROR-%s"), m_index, res.getStatusText());
		return res;
	}
}

} // namespace cz
