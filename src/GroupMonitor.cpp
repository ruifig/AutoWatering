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
	raiseEvent(MotorStarted(m_index));
}

void GroupMonitor::turnMotorOff()
{
	m_ctx.ioExpander.digitalWrite(m_motorPin1, LOW);
	m_ctx.ioExpander.digitalWrite(m_motorPin2, LOW);
	raiseEvent(MotorStopped(m_index));
}

float GroupMonitor::tick(float deltaSeconds)
{
	m_totalTime += deltaSeconds;
	
	return 0.2f;
}

void GroupMonitor::onEvent(const Event& evt)
{
}
	
} // namespace cz
