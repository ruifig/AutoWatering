/**
 * XPT2046 wrapper
 */

#pragma once

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wignored-qualifiers"
	#include "SPI.h"
	#include "XPT2046_Touchscreen.h"
#pragma GCC diagnostic pop

#include "../utility/PinTypes.h"
#include "TouchScreenController.h"

namespace cz
{

class MyXPT2046 : public TouchScreenController
{
public:
	MyXPT2046(GraphicsInterface& display, MCUPin cspin, MCUPin irqpin, const TouchCalibrationData* calibrationData)
		: TouchScreenController(display, calibrationData)
		, m_ts(cspin.raw, irqpin.raw)
	{
	}

	void begin();

	virtual RawTouchPoint getRawPoint() override;

	virtual bool isTouched() override
	{
		return m_ts.touched();
	}

private:
	mutable XPT2046_Touchscreen m_ts;
};

} // namespace cz
