#include <Arduino.h>

#include "BatteryLifeCalculator.h"

namespace cz
{

BatteryLifeCalculator::BatteryLifeCalculator(MCUPin ADCPin, int R1, int R2, float VBatMin, float VBatMax, float correctionScale = 1.07f)
	: m_ADCPin(ADCPin.raw)
	, m_R1(R1)
	, m_R2(R2)
	, m_VBatMin(VBatMin)
	, m_VBatMax(VBatMax)
	, m_correctionScale(correctionScale)
{
}

int BatteryLifeCalculator::readBatteryLife(float* batteryVoltage)
{
	float Vs = readBatteryVoltage();
	if (batteryVoltage)
	{
		*batteryVoltage = Vs;
	}

	int perc = static_cast<int>(estimate_soc(Vs));
	return perc;
}

float BatteryLifeCalculator::estimate_soc(float voltage)
{
	float soc;

	if (voltage <= 3.0)
	{
		soc = 0;
	}
	else if (voltage <= 3.4)
	{
		soc = 50 * (voltage - 3.0);
	}
	else if (voltage <= 4.0)
	{
		soc = 20 + 60 * (voltage - 3.4) / 0.6;
	}
	else if (voltage <= 4.2)
	{
		soc = 80 + 20 * (voltage - 4.0) / 0.2;
	}
	else
	{
		soc = 100;
	}

	return soc;
}

float BatteryLifeCalculator::readBatteryVoltage(int numSamples)
{
	int adcVal = sampleADC(m_ADCPin, numSamples);
	
	// I could not figure out why the reading is consistently lower than what my multimeter and oscilloscope report,
	// but applying this scaling gets closer to the real value
	adcVal = static_cast<int>(adcVal * m_correctionScale);

	// Calculate the voltage we are reading from the adc pin
	static constexpr int adcMaxValue = (1 << m_adcBits) - 1;
	float Vout = ((float)adcVal/adcMaxValue) * m_MCUVoltage;

	// Now calculate the real voltage considering the voltage divider.
	// Given Vout = (Vs * R2) / (R1 + R2), we want to know Vs, so
	// Vs = (Vout * (R1 + R2)) / R2
	float Vs = (Vout * (m_R1 + m_R2)) / m_R2;
	return Vs;
}

int BatteryLifeCalculator::sampleADC(int pin, int numSamples)
{
	analogReadResolution(m_adcBits);
	pinMode(pin, INPUT);

	int adcVal = 0;
	for (int i = 0; i < numSamples; i++)
	{
		adcVal += analogRead(pin);
	}
	adcVal = adcVal / numSamples;
	return adcVal;
}

} // namespace cz
