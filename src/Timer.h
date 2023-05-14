#pragma once

#include <Arduino.h>

namespace cz
{

class Timer
{
  public:

	void begin()
	{
		m_previousMicros = micros();
	}

	/**
	 * Updates the internal time calculations. 
	 * It should be called at the beginning of every tick
	 */
	void update();

	/**
	 * Returns how many seconds have elased since the previous tick.
	 * This requires update() to be called at the beginning of the tick
	 */
	float getDeltaSeconds() const
	{
		return m_deltaSeconds;
	}

	struct RunningTime
	{
		int days;
		int hours;
		int minutes;
		int seconds;
	};

	/**
	 * Returns total running time.
	 * It requires update() to be called at the beginning of the tick
	 */
	RunningTime getRunningTime() const
	{
		return m_runningTime;
	}

	uint64_t getTotalMicros() const
	{
		return m_totalMicros;
	}

	float getTotalSeconds() const
	{
		return m_totalMicros / 1000000.0f;
	}

  private:
	unsigned long m_previousMicros = 0;
	float m_deltaSeconds = 0.0f;

	uint64_t m_totalMicros = 0;

	RunningTime m_runningTime;
};

}

