#include "GroupMonitor.h"
#include "Context.h"

namespace cz
{

TSemaphoreQueue<uint8_t, MAX_NUM_PAIRS, 3> GroupMonitor::ms_semaphoreQueue;

GroupMonitor::GroupMonitor(uint8_t index, IOExpanderPinInstance motorPin)
	: m_index(index)
	, m_motorPin(motorPin)
	, m_sensorValidReadingSinceLastShot(0)
{
}

void GroupMonitor::begin()
{
	m_motorPin.pinMode(OUTPUT);
	m_motorPin.digitalWrite(LOW);
}

void GroupMonitor::doShot()
{
	CZ_LOG(logDefault, Log, F("Initiating user requested shot for group %d"), m_index);
	// If we are already trying to turn the motor on (it's waiting in queue), then do nothing
	if (!m_inSemaphoreQueue)
	{
		tryTurnMotorOn(false);
	}
}

void GroupMonitor::removeFromSemaphoreQueue()
{
	ms_semaphoreQueue.release(m_index);
	m_inSemaphoreQueue = false;
}

bool GroupMonitor::tryTurnMotorOn(bool registerInterest)
{
	GroupData& data = gCtx.data.getGroupData(m_index);
	if (data.isMotorOn())
	{
		CZ_LOG(logDefault, Warning, F("Request to turn motor on for group %d ignored because motor was already on"), m_index);
		return false;
	}

	if (registerInterest)
	{
		// Note: Intentionally not doing m_inSemaphoreQueue = registerInterest, because we don't want to accidentely change from true to false
		m_inSemaphoreQueue = true;
	}

	if (ms_semaphoreQueue.tryAcquire(m_index, registerInterest))
	{
		m_motorPin.digitalWrite(HIGH);
		data.setMotorState(true);
		m_motorOffCountdown = data.getShotDuration();
		m_sensorValidReadingSinceLastShot = false;
		return true;
	}
	else
	{
		if (!m_inSemaphoreQueue)
		{
			CZ_LOG(logDefault, Log, F("Request to turn motor on for group %d failed."), m_index)
		}
		return false;
	}
}

void GroupMonitor::turnMotorOff()
{
	m_motorPin.digitalWrite(LOW);
	GroupData& data = gCtx.data.getGroupData(m_index);
	data.setMotorState(false);
	removeFromSemaphoreQueue();
}

float GroupMonitor::tick(float deltaSeconds)
{
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
	else if (m_motorOffCountdown <= -MINIMUM_TIME_BETWEEN_MOTOR_ON)
	{
		if (data.isRunning() && m_sensorValidReadingSinceLastShot && m_lastValidReading.meanValue > data.getThresholdValue())
		{
			tryTurnMotorOn(true);
		}
		else
		{
			// If we are at a point where the motor is ready to turn on, but for some reason it didn't (group not running, no sensor reading since last shot, etc)
			// then we need to make sure we release our slot from the semaphore queue
			if (m_inSemaphoreQueue)
			{
				removeFromSemaphoreQueue();
			}
		}
	}
	else // m_motorOffCountdown is in the ]-MINIMUM_TIME_BETWEEN_MOTOR_ON, 0] range
	{
		// keep counting down until we get to <= -MINIMUM_TIME_BETWEEN_MOTOR_ON
		m_motorOffCountdown -= deltaSeconds;
	}

	return 0.2f;
}

void GroupMonitor::onEvent(const Event& evt)
{
	switch(evt.type)
	{
		case Event::ConfigLoad:
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

		default:
		break;
	}
}
	
} // namespace cz

