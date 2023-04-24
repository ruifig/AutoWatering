/**
 * XPT2046 wrapper
 */

#pragma once

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wignored-qualifiers"
	#include "SPI.h"
	#include "details/XPT2046_Touchscreen_Driver.h"
#pragma GCC diagnostic pop

#include "TouchScreenController.h"

namespace cz
{

class XPT2046 : public TouchScreenController
{
public:

	/**
	 * \param display Minimal graphics interface needed by the touch controller
	 * \param cspin Chip Select pin
	 * \param irqpin PEN/IRQ pin. If this is is 255, then no IRQ pin is used and SPI calls are always made.
	 * \param calibrationData Calibration data to use. You should probably run the calibrate method to get the best data for you specific display.
	 * 
	 */
	XPT2046(TouchScreenController_GraphicsInterface& display, uint8_t cspin, uint8_t irqpin, const TouchCalibrationData* calibrationData)
		: TouchScreenController(display, calibrationData)
		, m_ts(cspin, irqpin)
	{
	}

	void begin(SPIClass& wspi);

	virtual void updateState() override;

	virtual TouchState getState() override
	{
		return m_state;
	} 

	virtual RawTouchPoint getRawPoint() override
	{
		return m_lastRawPoint;
	}

	virtual TouchPoint getPoint() override
	{
		return m_lastPoint;
	}

	virtual unsigned irqCounter() const
	{
		return m_ts.irqCounter;
	}

private:
	XPT2046_Touchscreen_Driver m_ts;
	TouchState m_state = TouchState::Idle;
	TouchPoint m_lastPoint;
	RawTouchPoint m_lastRawPoint;
};

} // namespace cz
