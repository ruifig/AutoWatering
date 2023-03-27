#include "TouchScreenController.h"
#include "crazygaze/micromuc/Logging.h"
#include "crazygaze/micromuc/MathUtils.h"

namespace cz
{

TouchScreenController::TouchScreenController(TouchScreenController_GraphicsInterface& display, const TouchCalibrationData* calibrationData)
	: m_display(display)
{

	if (calibrationData)
	{
		m_touchCalibration_x0 = calibrationData->b0;
		m_touchCalibration_x1 = calibrationData->b1;
		m_touchCalibration_y0 = calibrationData->b2;
		m_touchCalibration_y1 = calibrationData->b3;

		if (m_touchCalibration_x0 == 0)
			m_touchCalibration_x0 = 1;
		if (m_touchCalibration_x1 == 0)
			m_touchCalibration_x1 = 1;
		if (m_touchCalibration_y0 == 0)
			m_touchCalibration_y0 = 1;
		if (m_touchCalibration_y1 == 0)
			m_touchCalibration_y1 = 1;

		m_touchCalibration_rotate = calibrationData->b4 & 0x01;
		m_touchCalibration_invert_x = calibrationData->b4 & 0x02;
		m_touchCalibration_invert_y = calibrationData->b4 & 0x04;
	}
}

TouchPoint TouchScreenController::convertRawToScreen(const RawTouchPoint& point) const
{
	uint16_t x_tmp = point.x;
	uint16_t y_tmp = point.y;
	uint16_t xx, yy;

	int16_t width = m_display.width();
	int16_t height = m_display.height();

	if (!m_touchCalibration_rotate)
	{
		xx = (x_tmp - m_touchCalibration_x0) * width / m_touchCalibration_x1;
		yy = (y_tmp - m_touchCalibration_y0) * height / m_touchCalibration_y1;
		if (m_touchCalibration_invert_x)
			xx = width - xx;
		if (m_touchCalibration_invert_y)
			yy = height - yy;
	}
	else
	{
		xx = (y_tmp - m_touchCalibration_x0) * width / m_touchCalibration_x1;
		yy = (x_tmp - m_touchCalibration_y0) * height / m_touchCalibration_y1;
		if (m_touchCalibration_invert_x)
			xx = width - xx;
		if (m_touchCalibration_invert_y)
			yy = height - yy;
	}

	//
	// clamp x and y to screen size, since then can be off due to calibration not being 100% accurate
	//
	TouchPoint ret(
		clamp<int16_t>(xx, 0, width-1),
		clamp<int16_t>(yy, 0, height-1),
		point.z);
	return ret;
}

TouchCalibrationData TouchScreenController::calibrate()
{
	constexpr uint16_t colour_fg = 0x0000; // Black
	constexpr uint16_t colour_bg = 0xFFFF; // White

	constexpr uint8_t size = 15; // Rectangle size

	int16_t values[] = {0, 0, 0, 0, 0, 0, 0, 0};
	int16_t height = m_display.height();
	int16_t width = m_display.width();
	CZ_LOG(logDefault, Log, "Screen: width=%d, height=%d", width, height);

	// Call once to kickstart anything
	getRawPoint();

	for (uint8_t i = 0; i < 4; i++)
	{
		if (i == 5)
			break; // used to clear the arrows

		switch (i)
		{
		case 0: // up left
			m_display.drawLine(0, 0, 0, size, colour_fg);
			m_display.drawLine(0, 0, size, 0, colour_fg);
			m_display.drawLine(0, 0, size, size, colour_fg);
			break;
		case 1: // bot left
			m_display.drawLine(0, height - size - 1, 0, height - 1, colour_fg);
			m_display.drawLine(0, height - 1, size, height - 1, colour_fg);
			m_display.drawLine(size, height - size - 1, 0, height - 1, colour_fg);
			break;
		case 2: // up right
			m_display.drawLine(width - size - 1, 0, width - 1, 0, colour_fg);
			m_display.drawLine(width - size - 1, size, width - 1, 0, colour_fg);
			m_display.drawLine(width - 1, size, width - 1, 0, colour_fg);
			break;
		case 3: // bot right
			m_display.drawLine(width - size - 1, height - size - 1, width - 1, height - 1, colour_fg);
			m_display.drawLine(width - 1, height - 1 - size, width - 1, height - 1, colour_fg);
			m_display.drawLine(width - 1 - size, height - 1, width - 1, height - 1, colour_fg);
			break;
		}

		while(getState() != TouchState::Idle) { updateState(); }
		while(getState() != TouchState::Touching) { updateState(); }
		while(getState() != TouchState::Released) { updateState(); }

		RawTouchPoint tmp = getRawPoint();
		CZ_LOG(logDefault, Log, "RawTouchPoint: x=%u, y=%u, z=%u", tmp.x, tmp.y, tmp.z);
		values[i * 2] = tmp.x;
		values[i * 2 + 1] = tmp.y;

		// quickly fill inverted, for visual feedback that we moved to the next step
		// Reset TFT_eSPI frequency, because touch is using the same SPI ports.
		m_display.fillScreen(colour_fg);
		m_display.fillScreen(colour_bg);
	}

	while(getState() != TouchState::Idle) { updateState(); }

	// from case 0 to case 1, the y value changed.
	// If the measured delta of the touch x axis is bigger than the delta of the y axis, the touch and TFT axes are switched.
	m_touchCalibration_rotate = false;
	if (abs(values[0] - values[2]) > abs(values[1] - values[3]))
	{
		m_touchCalibration_rotate = true;
		m_touchCalibration_x0 = (values[1] + values[3]) / 2; // calc min x
		m_touchCalibration_x1 = (values[5] + values[7]) / 2; // calc max x
		m_touchCalibration_y0 = (values[0] + values[4]) / 2; // calc min y
		m_touchCalibration_y1 = (values[2] + values[6]) / 2; // calc max y
	}
	else
	{
		m_touchCalibration_x0 = (values[0] + values[2]) / 2; // calc min x
		m_touchCalibration_x1 = (values[4] + values[6]) / 2; // calc max x
		m_touchCalibration_y0 = (values[1] + values[5]) / 2; // calc min y
		m_touchCalibration_y1 = (values[3] + values[7]) / 2; // calc max y
	}

	// in addition, the touch screen axis could be in the opposite direction of the TFT axis
	m_touchCalibration_invert_x = false;
	if (m_touchCalibration_x0 > m_touchCalibration_x1)
	{
		values[0] = m_touchCalibration_x0;
		m_touchCalibration_x0 = m_touchCalibration_x1;
		m_touchCalibration_x1 = values[0];
		m_touchCalibration_invert_x = true;
	}
	m_touchCalibration_invert_y = false;
	if (m_touchCalibration_y0 > m_touchCalibration_y1)
	{
		values[0] = m_touchCalibration_y0;
		m_touchCalibration_y0 = m_touchCalibration_y1;
		m_touchCalibration_y1 = values[0];
		m_touchCalibration_invert_y = true;
	}

	// pre calculate
	m_touchCalibration_x1 -= m_touchCalibration_x0;
	m_touchCalibration_y1 -= m_touchCalibration_y0;

	if (m_touchCalibration_x0 == 0)
		m_touchCalibration_x0 = 1;
	if (m_touchCalibration_x1 == 0)
		m_touchCalibration_x1 = 1;
	if (m_touchCalibration_y0 == 0)
		m_touchCalibration_y0 = 1;
	if (m_touchCalibration_y1 == 0)
		m_touchCalibration_y1 = 1;

	TouchCalibrationData ret;
	ret.b0 = m_touchCalibration_x0;
	ret.b1 = m_touchCalibration_x1;
	ret.b2 = m_touchCalibration_y0;
	ret.b3 = m_touchCalibration_y1;
	ret.b4 = m_touchCalibration_rotate | (m_touchCalibration_invert_x << 1) | (m_touchCalibration_invert_y << 2);
	CZ_LOG(logDefault, Log, "Calibration data: uint16_t calibrationData[5] = {%u, %u, %u, %u, %u};"
		, ret.b0, ret.b1, ret.b2, ret.b3, ret.b4);
	return ret;
}

} // namespace cz
