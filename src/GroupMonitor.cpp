#include "GroupMonitor.h"

namespace cz
{

GroupMonitor::GroupMonitor(Context& ctx, uint8_t index, IOExpanderPin motorPin1, IOExpanderPin motorPin2)
	: m_ctx(ctx)
	, m_index(index)
	, m_motorPin1(motorPin1)
	, m_motorPin2(motorPin2)
{
}

void GroupMonitor::begin()
{
	m_ctx.ioExpander.pinMode(m_motorPin1, OUTPUT);
	m_ctx.ioExpander.pinMode(m_motorPin2, OUTPUT);

	m_ctx.ioExpander.digitalWrite(m_motorPin1, LOW);
	m_ctx.ioExpander.digitalWrite(m_motorPin2, LOW);
}

void GroupMonitor::turnMotorOn(bool direction)
{
	m_ctx.ioExpander.digitalWrite(m_motorPin1, direction ? LOW : HIGH);
	m_ctx.ioExpander.digitalWrite(m_motorPin2, direction ? HIGH : LOW);
	GroupData& data = m_ctx.data.getGroupData(m_index);
	data.setMotorState(true);
	m_motorOffCountdown = data.getShotDuration();
}

void GroupMonitor::turnMotorOff()
{
	m_ctx.ioExpander.digitalWrite(m_motorPin1, LOW);
	m_ctx.ioExpander.digitalWrite(m_motorPin2, LOW);
	GroupData& data = m_ctx.data.getGroupData(m_index);
	data.setMotorState(false);
}

float GroupMonitor::tick(float deltaSeconds)
{
	GroupData& data = m_ctx.data.getGroupData(m_index);

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
		if (data.getCurrentValue() > data.getThresholdValue())
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


#warning Was here making GroupMonitor have states so we are sure it turns the motor OFF if the user disables the group
void GroupMonitor::onEvent(const Event& evt)
{
#if 0
	switch(evt.type)
	{
		case Event::ConfigLoad:
		break;

		case Event::Group:
		{
			const GroupEvent& e = static_cast<const GroupEvent&>(evt);
		}
		break;

		break;
	}
#endif
}
	
} // namespace cz
