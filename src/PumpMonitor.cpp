#include "PumpMonitor.h"
#include "Context.h"
#include <crazygaze/micromuc/StringUtils.h>
#include "crazygaze/micromuc/Profiler.h"

namespace cz
{

PumpMonitor::SemaphoreQueue PumpMonitor::ms_semaphoreQueue;

PumpMonitor::PumpMonitor(uint8_t index, DigitalOutputPin& motorPin)
	: m_index(index)
	, m_motorPin(motorPin)
	, m_sensorValidReadingSinceLastShot(0)
	, m_queueHandle(ms_semaphoreQueue.createHandle())
{
	// We only start ticking when we get a ConfigReady event
	stopTicking();
}

const char* PumpMonitor::getName() const
{
	return formatString("PumpMonitor%d", m_index);
}

bool PumpMonitor::initImpl()
{
	m_motorPin.write(PinStatus::LOW);
	return true;
}

void PumpMonitor::doShot()
{
	CZ_LOG(logDefault, Log, F("Initiating user requested shot for group %d"), m_index);
	// If we are already trying to turn the motor on (it's waiting in queue), then do nothing
	if (m_queueHandle.isActiveOrQueued())
	{
		CZ_LOG(logDefault, Log, F("Motor for group %d is already On or is queued to turn on."), m_index);
	}
	else
	{
		tryTurnMotorOn(false);
	}
}

bool PumpMonitor::tryTurnMotorOn(bool registerInterest)
{
	GroupData& data = gCtx.data.getGroupData(m_index);
	if (data.isMotorOn())
	{
		CZ_LOG(logDefault, Warning, F("Request to turn motor on for group %d ignored because motor was already on"), m_index);
		return false;
	}

	if (m_queueHandle.tryAcquire(registerInterest))
	{
		m_motorPin.write(PinStatus::HIGH);
		data.setMotorState(true);
		m_motorOffCountdown = data.getShotDuration();
		m_sensorValidReadingSinceLastShot = false;
		return true;
	}
	else
	{
		if (!m_queueHandle.isActiveOrQueued())
		{
			CZ_LOG(logDefault, Log, F("Request to turn motor on for group %d failed."), m_index)
		}
		return false;
	}
}

void PumpMonitor::turnMotorOff()
{
	GroupData& data = gCtx.data.getGroupData(m_index);
	if (data.isMotorOn())
	{
		m_motorPin.write(PinStatus::LOW);
		data.setMotorState(false);
		m_queueHandle.release();
	}
}

float PumpMonitor::tick(float deltaSeconds)
{
	PROFILE_SCOPE(F("PumpMonitor"));

	GroupData& data = gCtx.data.getGroupData(m_index);

	//
	// Even if the group is not running, we still need to tick and check if the motor is on, to handle
	// user initiated shots
	//

	if (m_motorOffCountdown > 0) // Motor is on
	{
		m_motorOffCountdown -= deltaSeconds;
		if (m_motorOffCountdown < 0)
		{
			turnMotorOff();
		}
	}
	else if (m_motorOffCountdown <= -AW_MINIMUM_TIME_BETWEEN_MOTOR_ON)
	{
		if (data.isRunning() && m_sensorValidReadingSinceLastShot && m_lastValidReading.meanValue > data.getThresholdValue())
		{
			tryTurnMotorOn(true);
		}
		else
		{
			// If we are at a point where the motor is ready to turn on, but for some reason it didn't (group not running, no sensor reading since last shot, etc)
			// then we need to make sure we release our slot from the semaphore queue
			m_queueHandle.release();
		}
	}
	else // m_motorOffCountdown is in the ]-AW_MINIMUM_TIME_BETWEEN_MOTOR_ON, 0] range
	{
		// keep counting down until we get to <= -AW_MINIMUM_TIME_BETWEEN_MOTOR_ON
		m_motorOffCountdown -= deltaSeconds;
	}

	return 0.2f;
}

void PumpMonitor::onEvent(const Event& evt)
{
	switch(evt.type)
	{
		case Event::ConfigReady:
			startTicking();
		break;

		case Event::SoilMoistureSensorReading:
		{
			const auto& e = static_cast<const SoilMoistureSensorReadingEvent&>(evt);
			// The system will still raise events for invalid readings, but we don't want to act. As-in, we don't want bad sensor readings
			// to end up turning the water on.
			if (e.index == m_index && e.reading.isValid())
			{
				m_sensorValidReadingSinceLastShot = true;
				m_lastValidReading = e.reading;
			}
		}
		break;

		case Event::GroupOnOff:
		{
			const GroupOnOffEvent& e = static_cast<const GroupOnOffEvent&>(evt);
			if (e.index == m_index && e.started == false)
			{
				turnMotorOff();
			}
		}
		break;

		case Event::WifiConnecting:
		{
			// Connecting to Wifi can take a long time, so we should turn off the motors if any are on
			turnMotorOff();
		}
		break;

		default:
		break;
	}
}

bool PumpMonitor::processCommand(const Command& cmd)
{
	if (cmd.is("motoroff"))
	{
		turnMotorOff();
		return true;
	}
	else if (cmd.is("motoron"))
	{
		doShot();
		return true;
	}

	return false;
}
	
} // namespace cz

