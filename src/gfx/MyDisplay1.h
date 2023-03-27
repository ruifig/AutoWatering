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
  //#include "SPI.h"
#pragma GCC diagnostic pop

#include "TFT_eSPI.h"
#include "GraphicsInterface.h"
#include "../utility/PinTypes.h"

namespace cz
{

class MyDisplay1 : public GraphicsInterface
{
  public:
	MyDisplay1();

	/*
	* Starts the display device, and returns true on success, or false if not found.
	* The detection method is simply based on drawing a few pixels and reading them back.
	*/
	bool begin();

	virtual int16_t height() override { return m_tft.height(); }
	virtual int16_t width() override { return m_tft.width(); }

	virtual void writePixel(int16_t x, int16_t y, Colour color) override { m_tft.drawPixel(x, y, color); }
	virtual void startWrite() override { m_tft.startWrite(); }
	virtual void endWrite() override { m_tft.endWrite(); }

	virtual void drawPixel(int16_t x, int16_t y, Colour color) override { m_tft.drawPixel(x, y, color); }

	virtual void fillScreen(Colour colour) override { m_tft.fillScreen(colour); }

	virtual void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, Colour color) override
	{
		m_tft.fillRect(x, y, w, h, color);
	}

	void fillRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, Colour color) override
	{
		m_tft.fillRoundRect(x, y, w, h, r, color);
	}

	virtual void drawRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, Colour color) override
	{
		m_tft.drawRoundRect(x, y, w, h, r, color);
	}
	virtual void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, Colour color) override
	{
		m_tft.drawRect(x, y, w, h, color);
	}
	virtual void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, Colour color) override
	{
		m_tft.drawLine(x0, y0, x1, y1, color);
	}
	virtual void drawFastVLine(int16_t x, int16_t y, int16_t h, Colour color) override
	{
		m_tft.drawFastVLine(x, y, h, color);
	}
	virtual void drawFastHLine(int16_t x, int16_t y, int16_t w, Colour color) override
	{
		m_tft.drawFastHLine(x, y, w, color);
	}

	//
	// Text rendering methods
	//

	virtual void setTextSize(uint8_t s) override { m_tft.setTextSize(s); }
	virtual void setFont(const GFXfont* font) override
	{
		m_tft.setFreeFont(font);
	}

	virtual GFXfont* getGfxFont() override
	{
		return m_tft.getGfxFont();
	}

	virtual void setTextColor(Colour c) override
	{
		m_textColour = c;
		m_textBkgColour = c;
		m_tft.setTextColor(c);
	}

	virtual void setTextColor(Colour c, Colour bg) override
	{
		m_textColour = c;
		m_textBkgColour = bg;
		m_tft.setTextColor(c, bg);
	}

	virtual Colour getTextColor() const override
	{
		return m_textColour;
	}

	virtual Colour getTextBkgColor() const
	{
		return m_textBkgColour;
	}

	virtual void setCursor(int16_t x, int16_t y) override
	{
		m_tft.setCursor(x, y);
	}

	virtual void print(const char* str) override
	{
		m_tft.print(str);
	}

	virtual void print(const __FlashStringHelper* str) override
	{
		m_tft.print(str);
	}

	virtual void getTextBounds(const char *str, int16_t x, int16_t y,
									int16_t *x1, int16_t *y1, uint16_t *w,
									uint16_t *h) override
	{
		*x1 = x;
		*y1 = y;
		*w = m_tft.textWidth(str);
		*h = m_tft.fontHeight();
	}

	virtual void getTextBounds(const __FlashStringHelper *str, int16_t x, int16_t y,
									int16_t *x1, int16_t *y1, uint16_t *w,
									uint16_t *h) override
	{
		*x1 = x;
		*y1 = y;
		*w = m_tft.textWidth(str);
		*h = m_tft.fontHeight();
	}

	/**
	 * This should be called when adding a new screen, so you know what calibration data to use.
	 * This logs calibration data what you then need to hardcode in this class begin() method.
	 */
	void doTouchCalibration();

	virtual void setBacklightBrightness(uint8_t brightness) override;

	TFT_eSPI& getTFT_eSPI()
	{
		return m_tft;
	}

  private:

	uint32_t readRegister(uint8_t reg, int16_t bytes, uint8_t index);

	void logProperties();
	MCUPin m_bkpin;

	Colour m_textColour;
	Colour m_textBkgColour;

	// Wrapping, so I can access protected members
	class My_TFT_eSPI : public TFT_eSPI
	{
	  public:
		using TFT_eSPI::TFT_eSPI;
		GFXfont* getGfxFont() { return gfxFont; }
	} m_tft;
};

} // namespace cz

