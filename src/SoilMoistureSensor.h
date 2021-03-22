#pragma once

#include <Arduino.h>
#include <assert.h>

namespace cz
{
// Forward declarations
struct ProgramData;

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

	SoilMoistureSensor()
	{
	}

	/**
	 * @param data Program data to use
	 * @param index Sensor's index within the program data
	 * @param vinPin What Arduino pin we are using to power this sensor
	 * @param dataPin What Arduino analog pin we are using to read the sensor
	 */
	SoilMoistureSensor(ProgramData& data, uint8_t index, uint8_t vinPin, uint8_t dataPin);

	// Disable copying
	SoilMoistureSensor(const SoilMoistureSensor&) = delete;
	const SoilMoistureSensor& operator=(const SoilMoistureSensor&) = delete;

	void setup(ProgramData& data, uint8_t index, uint8_t vinPin, uint8_t dataPin);
	float tick(float deltaSeconds);

  private:
	enum class State : uint8_t
	{
		Off,
		PoweringUp,
		Reading
	};

	ProgramData* m_data = nullptr;
	uint8_t m_index;
	uint8_t m_vinPin;
	uint8_t m_dataPin;
	State m_state = State::Off;
	float m_timeInState = 0;
};

}  // namespace cz
