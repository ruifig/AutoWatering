/**
 * Puts together everything needed to have a working display and touchpad
 * 
 * Display module: https://smile.amazon.co.uk/gp/product/B07QJW73M3
 * 
 * Driver: ILI9341
 * Protocol: SPI
 * Touch controller: XPT2046
 * Resolution 320x240
 * Extras:
 * 		Backlight brightness control
 */

#pragma once

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wignored-qualifiers"
  #include "SPI.h"
  #include "Adafruit_ILI9341.h"
#pragma GCC diagnostic pop

#include "../utility/PinTypes.h"
#include "GraphicsInterface.h"

namespace cz
{

class MyDisplay1 : public GraphicsInterface
{
  public:
	MyDisplay1(MCUPin displayCS, MCUPin displayDC, MCUPin displayBacklight);

	void begin();

	virtual int16_t height() const override { return m_tft.height(); }
	virtual int16_t width() const override { return m_tft.width(); }

	virtual void fillScreen(Colour colour) override
	{
		m_tft.fillScreen(colour);
	}

	virtual void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, Colour color) override
	{
		m_tft.fillRect(x, y, w, h, color);
	}

	virtual void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, Colour color) override
	{
		m_tft.drawLine(x0, y0, x1, y1, color);
	}

	//
	// Text rendering methods
	//
	virtual void setFont(const GFXfont* font) override
	{
		m_tft.setFont(font);
	}

	virtual void setTextColor(Colour c) override
	{
		m_tft.setTextColor(c);
	}

	virtual void setTextColor(Colour c, Colour bg) override
	{
		m_tft.setTextColor(c, bg);
	}

	virtual void setCursor(int16_t x, int16_t y) override
	{
		m_tft.setCursor(x, y);
	}

	virtual void print(const char* str) override
	{
		m_tft.print(str);
	}

	/**
	 * This should be called when adding a new screen, so you know what calibration data to use.
	 * This logs calibration data what you then need to hardcode in this class begin() method.
	 */
	void doTouchCalibration();

	virtual void setBacklightBrightness(uint8_t brightness) override;

  private:

	void logProperties();
	MCUPin m_bkpin;
	Adafruit_ILI9341 m_tft;
};

} // namespace cz

