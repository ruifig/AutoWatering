#pragma once

#include "PinTypes.h"
#include "../Config/Config.h"

namespace cz
{

/**
 * Calculates battery life (0..100%) simply based on the battery voltage
 * TODO: Use a smoothing function such as specified in: https://stackoverflow.com/questions/5516641/smoothing-function-for-battery-voltage-display-to-reduce-spikes-in-embedded-syst
 * Also, taking the battery voltage curve into consideration:
 *		https://electronics.stackexchange.com/questions/538632/battery-life-calculation-depending-voltage-levels
 * Batteries I bought: https://www.overlander.co.uk/pub/media/datasheets/2600mAh_Li-Ion_Cell_Specification.pdf
 */
class BatteryLifeCalculator
{

  public:

	/*
	* A good voltage divider to calculate what resistors you need for a given battery voltage rang: https://ohmslawcalculator.com/voltage-divider-calculator
	*
	* R1 - Value in ohm
	* R2 - Value in ohm
	* VBatMin - Voltage we should consider the battery to be 0% (e.g: For a 18650 battery, probably around 2.5v to 2.7v depending on the manufacturer)
	* VBatMax - Voltage we should consider the battery to be 100% (e.g: For a 18650 battery, this is 4.2v)
	*/
/*
A typical 18650 lithium-ion battery has a non-linear voltage discharge curve. While it's difficult to provide a generic formula for all 18650 batteries, you can approximate the voltage curve using a piecewise linear or polynomial function. However, this approximation will not be very accurate, as the actual discharge curve varies depending on the battery's specific chemistry, manufacturer, and capacity.

Here's a simple piecewise linear approximation for a 18650 battery with a voltage range between 3.0V (discharged) and 4.2V (fully charged):

Between 3.0V and 3.4V: Assume a linear relationship between voltage and state of charge (SoC), with SoC ranging from 0% to 20%. The slope in this region is 50%/0.4V.
Between 3.4V and 4.0V: Assume a linear relationship between voltage and SoC, with SoC ranging from 20% to 80%. The slope in this region is 60%/0.6V.
Between 4.0V and 4.2V: Assume a linear relationship between voltage and SoC, with SoC ranging from 80% to 100%. The slope in this region is 20%/0.2V.
With this approximation, you can estimate the SoC based on the measured battery voltage:

float estimate_soc(float voltage) {
    float soc;

    if (voltage <= 3.0) {
        soc = 0;
    } else if (voltage <= 3.4) {
        soc = 50 * (voltage - 3.0);
    } else if (voltage <= 4.0) {
        soc = 20 + 60 * (voltage - 3.4) / 0.6;
    } else if (voltage <= 4.2) {
        soc = 80 + 20 * (voltage - 4.0) / 0.2;
    } else {
        soc = 100;
    }

    return soc;
}

Please note that this is a very simplified approximation and may not provide accurate results for your specific 18650 battery. The best approach would be to obtain the discharge curve from the battery manufacturer or perform discharge tests on the specific battery model you're using. With the actual discharge curve, you can create a lookup table or fit a polynomial function to estimate the SoC more accurately.

It's also important to consider other factors that can affect the battery voltage, such as temperature, load, and battery aging, as these can impact the accuracy of the estimation.

*/
	BatteryLifeCalculator(MCUPin ADCPin, int R1, int R2, float VBatMin, float VBatMax, float correctionScale);

	/**
	 * Returns the battery life in percentage (0..100)
	 */
	int readBatteryLife(float* batteryVoltage = nullptr);

  private:

	//
	// Estimate battery remaining life simply based on a voltage reading and the typical voltage curve of a 18650 battery
	//
	float estimate_soc(float voltage);
	float readBatteryVoltage(int numSamples = 200);
	int sampleADC(int pin, int numSamples);

	static constexpr float m_MCUVoltage = 3.3f;
	static constexpr int m_adcBits = ADC_NUM_BITS;
	int m_ADCPin;
	int m_R1;
	int m_R2;
	float m_VBatMin;
	float m_VBatMax;
	float m_correctionScale;
};

} // namespace cz
