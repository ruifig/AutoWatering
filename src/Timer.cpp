
#include "Timer.h"

namespace cz
{

void Timer::update()
{
	unsigned long nowMicros = micros();
	// NOTE: If using subtraction, there is no need to handle wrap around
	// See: https://arduino.stackexchange.com/questions/33572/arduino-countdown-without-using-delay/33577#33577
	m_deltaSeconds = (nowMicros - m_previousMicros) / 1000000.0f;
	m_previousMicros = nowMicros;
	m_totalSeconds += m_deltaSeconds;


	//
	// Update running totals
	//
	{
		uint32_t secs = m_totalSeconds;

		m_runningTime.days = secs / 86400;
		secs -= m_runningTime.days * 86400;

		m_runningTime.hours = secs / 3600;
		secs -= m_runningTime.hours * 3600;

		m_runningTime.minutes = secs / 60;
		secs -= m_runningTime.minutes * 60;

		m_runningTime.seconds = secs;
	}

}


}
