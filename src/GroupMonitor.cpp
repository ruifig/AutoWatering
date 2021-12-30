#include "GroupMonitor.h"
#include "Context.h"

namespace cz
{

GroupMonitor::GroupMonitor(uint8_t index, IOExpanderPin motorPin1, IOExpanderPin motorPin2)
	: m_index(index)
	, m_motorPin1(motorPin1)
	, m_motorPin2(motorPin2)
	, m_sensorReadingSinceLastShot(0)
{
}

void GroupMonitor::begin()
{
	gCtx.ioExpander.pinMode(m_motorPin1, OUTPUT);
	gCtx.ioExpander.pinMode(m_motorPin2, OUTPUT);

	gCtx.ioExpander.digitalWrite(m_motorPin1, LOW);
	gCtx.ioExpander.digitalWrite(m_motorPin2, LOW);
}

void GroupMonitor::doShot()
{
	CZ_LOG(logDefault, Log, F("Initiating user requested shot for group %d"), m_index);
	turnMotorOn(true);
}

void GroupMonitor::turnMotorOn(bool direction)
{
	GroupData& data = gCtx.data.getGroupData(m_index);
	if (data.isMotorOn())
	{
		CZ_LOG(logDefault, Warning, F("Request to turn motor on for group %d ignored because motor was already on"), m_index);
		return;
	}

	gCtx.ioExpander.digitalWrite(m_motorPin1, direction ? LOW : HIGH);
	gCtx.ioExpander.digitalWrite(m_motorPin2, direction ? HIGH : LOW);
	data.setMotorState(true);
	m_motorOffCountdown = data.getShotDuration();
	m_sensorReadingSinceLastShot = false;
}

void GroupMonitor::turnMotorOff()
{
	gCtx.ioExpander.digitalWrite(m_motorPin1, LOW);
	gCtx.ioExpander.digitalWrite(m_motorPin2, LOW);
	GroupData& data = gCtx.data.getGroupData(m_index);
	data.setMotorState(false);
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
		if (data.isRunning() && m_sensorReadingSinceLastShot && data.getCurrentValue() > data.getThresholdValue())
		{
			turnMotorOn(true);
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
			GroupData& data = gCtx.data.getGroupData(m_index);
			if (data.isRunning())
			{
				const auto& e = static_cast<const SoilMoistureSensorReadingEvent&>(evt);
				if (e.index == m_index && !e.calibrating)
				{
					m_sensorReadingSinceLastShot = true;
				}
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
