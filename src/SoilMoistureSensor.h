#pragma once

#include <Arduino.h>
#include <assert.h>

class RelayModule
{
public:
	void setup(const uint8_t* pinIn, int numChannels);

	void setInput(uint8_t idx, uint8_t state);

#if 0
	void signalHigh(uint8_t idx);
	void signalLow(uint8_t idx);
	void signal(uint8_t idx, unsigned long duration);
#endif

private:
	static constexpr int m_maxChannels = 4;
	uint8_t m_pinIns[m_maxChannels];
	uint8_t m_numChannels = 0;
};

class SoilMoistureSensor
{
public:
	SoilMoistureSensor();

	// Disable copying
	SoilMoistureSensor(const SoilMoistureSensor&) = delete;
	const SoilMoistureSensor& operator=(const SoilMoistureSensor&) = delete;

	void setup(uint8_t m_id, uint8_t vinPin, uint8_t dataPin);
	void tick();

	uint8_t readValue();
	int getLastPinValue()
	{
		return m_lastPinValue;
	}

private:

	int m_airValue = 513;
	int m_waterValue = 512;
	int m_lastPinValue = 0;
	uint8_t m_id;
	uint8_t m_vinPin;
	uint8_t m_dataPin;
};



