#pragma once

#include <Arduino.h>
#include <assert.h>
#include "Context.h"

namespace cz
{

/**
 * Capacitive Soil Moisture Sensor: https://www.amazon.co.uk/gp/product/B08GC5KT4T
 *
 * These sensor can have a problem where their response to change is very slow. The following links explain how to fix
 * it: https://forum.arduino.cc/index.php?topic=597585.0 https://www.youtube.com/watch?v=QGCrtXf8YSs
 *
 * Setup:
 * - The sensor Vin should be connected to an Arduino digital pin. This is required so that the sensor is only powered
 * up when we need to make a reading. This saves power.
 */
class SoilMoistureSensor
{
  public:

	/**
	 * @param ctx program context
	 * @param index Sensor's index within the program data
	 * @param vinPin What io expander pin we are using to power this sensor
	 * @param dataPin What multiplexer pin we are using to read the sensor
	 */
	SoilMoistureSensor(Context& ctx, uint8_t index, IOExpanderPin vinPin, MultiplexerPin dataPin);

	// Disable copying
	SoilMoistureSensor(const SoilMoistureSensor&) = delete;
	const SoilMoistureSensor& operator=(const SoilMoistureSensor&) = delete;

	void setup();
	float tick(float deltaSeconds);

  private:
	enum class State : uint8_t
	{
		Off,
		PoweringUp,
		Reading
	};

	Context& m_ctx;
	uint8_t m_index;
	IOExpanderPin m_vinPin;
	MultiplexerPin m_dataPin;
	State m_state = State::Off;
	float m_timeInState = 0;
};

}  // namespace cz
