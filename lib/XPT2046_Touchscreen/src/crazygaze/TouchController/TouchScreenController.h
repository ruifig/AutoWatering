#pragma once

#include <Arduino.h>

namespace cz
{

struct RawTouchPoint
{
	RawTouchPoint() = default;
	RawTouchPoint(uint16_t x, uint16_t y, uint16_t z) : x(x), y(y), z(z) {}
	bool operator==(RawTouchPoint p) { return ((p.x == x) && (p.y == y) && (p.z == z)); }
	bool operator!=(RawTouchPoint p) { return ((p.x != x) || (p.y != y) || (p.z != z)); }
	uint16_t x = 0;
	uint16_t y = 0;
	uint16_t z = 0;
};

struct TouchPoint
{
	TouchPoint() = default;
	TouchPoint(int16_t x, int16_t y, int16_t z) : x(x), y(y), z(z) {}
	bool operator==(TouchPoint p) { return ((p.x == x) && (p.y == y) && (p.z == z)); }
	bool operator!=(TouchPoint p) { return ((p.x != x) || (p.y != y) || (p.z != z)); }
	int16_t x = 0;
	int16_t y = 0;
	int16_t z = 0;
};

struct TouchCalibrationData
{
	uint16_t b0;
	uint16_t b1;
	uint16_t b2;
	uint16_t b3;
	uint16_t b4;
};

class GraphicsInterface;

enum TouchState : uint8_t
{
	Idle,
	Touching,
	Released
};

// Minimal set of graphics methods that the touch controller needs to know about to perform point conversion and run calibration function (if called)
class TouchScreenController_GraphicsInterface
{
  public:
	virtual int16_t height() = 0;
	virtual int16_t width() = 0;
	virtual void fillScreen(uint16_t colour) = 0;
	virtual void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t colour) = 0;
};

class TouchScreenController
{
  public:

	/**
	 * \param display
	 * 		Display this touch controller is attached to. A touch controller needs an interface to the display,
	 * 		so it can convert raw touch data to screen coordinates, and also so it can run the calibration routine
	 * 		when requested.
	 * \param calibrationData
	 * 		Previously calibration data you got from running the calibrate() method, or nullptr if you don't
	 * 		have calibration data, in which case you should call calibrate after initializing everything and then
	 * 		hardcode the data into your project so you can feed it to the constructor.
	 */
	TouchScreenController(TouchScreenController_GraphicsInterface& display, const TouchCalibrationData* calibrationData);

	/**
	 * You should call this once per loop, before calling getState() or getPoint()
	 * This is helpful in detecting state transitions.
	 * 
	 * void loop()
	 * {
	 * 		ts.updateState();
	 * 		switch(ts.getState())
	 *		{
	 *			case TouchState::Idle:
	 *				// run code
	 *				break;
	 *			case TouchState::Touching:
	 *				// run code
	 *				break;
	 *			case TouchState::Released:
	 *				// run code
	 *				break;
	 *		}
	 * }
	 * 
	 * 
	 * Note that after a call to updateState you should always call getState() to act on any state changes. If you make multiple calls to updateState without checking the state, you risk missing state transitions, such as from Touching -> Released
	 * 
	 */
	virtual void updateState() = 0;

	/**
	 * 
	*/
	virtual TouchState getState() = 0;

	/**
	 * Returns the controller's current raw data.
	 * Typically this doesn't need to be called. It's useful only for troubleshooting.
	 */
	virtual RawTouchPoint getRawPoint() = 0;

	/**
	 * 
	 */
	virtual TouchPoint getPoint() = 0;

	/**
	 * Converts the given point in controller coordinates to screen coordinates
	 * Typically this doesn't need to be called, and you should use getPoint() which will give you ready to use
	 * screen coordinates.
	 * 
	 * \param point Raw data point to convert to screen coordinates
	 * \return Point in screen coordinates.
	 */
	TouchPoint convertRawToScreen(const RawTouchPoint& point) const;

	/**
	 * Runs a calibration routine, where you are expected to touch the corners, so it can calculate the right
	 * parameters to convert from the controller's raw data to screen coordinates.
	 * 
	 * Typically this only needs to be be called once during development, when using a given screen for the first time.
	 * It will log and return the calibration data, that in turn you need to feed to the constructor.
	 * Note that this also changes the current calibration data being used.
	 */
	TouchCalibrationData calibrate();

  private:

	TouchScreenController_GraphicsInterface& m_display;
	uint16_t m_touchCalibration_x0 = 300;
	uint16_t m_touchCalibration_x1 = 3600;
	uint16_t m_touchCalibration_y0 = 300;
	uint16_t m_touchCalibration_y1 = 3600;

	uint8_t m_touchCalibration_rotate = 1;
	uint8_t m_touchCalibration_invert_x = 2;
	uint8_t m_touchCalibration_invert_y = 0;
};

} // namespace cz