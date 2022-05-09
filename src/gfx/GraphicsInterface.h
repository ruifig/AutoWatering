#pragma once

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wignored-qualifiers"
	#include "Adafruit_GFX.h"
#pragma GCC diagnostic pop

#include "Colour.h"
#include "GFXUtils.h"

namespace cz
{

class GraphicsInterface
{
  public:

	virtual int16_t height() const = 0;
	virtual int16_t width() const = 0;

	virtual void writePixel(int16_t x, int16_t y, Colour color) = 0;
	virtual void startWrite() = 0;
	virtual void endWrite() = 0;

	virtual void drawPixel(int16_t x, int16_t y, Colour color) = 0;

	virtual void fillScreen(Colour colour) = 0;

	virtual void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, Colour color) = 0;

	/**
	 * \brief   Draw a rounded rectangle with fill color
	 * \param    x   Top left corner x coordinate
	 * \param    y   Top left corner y coordinate
	 * \param    w   Width in pixels
	 * \param    h   Height in pixels
	 * \param    r   Radius of corner rounding
	 * \param    color 16-bit 5-6-5 Color to draw/fill with
	 */
	virtual void fillRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, Colour color) = 0;

	/**
	 * \brief   Draw a rounded rectangle with no fill color
	 * \param    x   Top left corner x coordinate
	 * \param    y   Top left corner y coordinate
	 * \param    w   Width in pixels
	 * \param    h   Height in pixels
	 * \param    r   Radius of corner rounding
	 * \param    color 16-bit 5-6-5 Color to draw with
	 */
	virtual void drawRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, Colour color) = 0;

	/**
	 * \brief   Draw a rectangle with no fill color
	 * \param    x   Top left corner x coordinate
	 * \param    y   Top left corner y coordinate
	 * \param    w   Width in pixels
	 * \param    h   Height in pixels
	 * \param    color 16-bit 5-6-5 Color to draw with
	 */
	virtual void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, Colour color) = 0;
	virtual void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, Colour color) = 0;
	virtual void drawFastVLine(int16_t x, int16_t y, int16_t h, Colour color) = 0;
	virtual void drawFastHLine(int16_t x, int16_t y, int16_t w, Colour color) = 0;
	void drawVLine(const VLine& line, Colour color)
	{
		drawFastVLine(line.p.x, line.p.y, line.height, color);
	}
	void drawHLine(const HLine& line, Colour color)
	{
		drawFastHLine(line.p.x, line.p.y, line.width, color);
	}

	//
	// Text rendering
	//

	/**
	 * @brief   Set text 'magnification' size. Each increase in s makes 1 pixel that much bigger.
	 * @param  s  Desired text size. 1 is default 6x8, 2 is 12x16, 3 is 18x24, etc
	 */
	virtual void setTextSize(uint8_t s) = 0;

	/**
	 * @brief   Set text 'magnification' size. Each increase in s makes 1 pixel that much bigger.
	 * @param  s_x  Desired text width magnification level in X-axis. 1 is default
	 * @param  s_y  Desired text width magnification level in Y-axis. 1 is default
	 */
	virtual void setTextSize(uint8_t s_x, uint8_t s_y) = 0;

	/**
	 * \param font Font to use, or NULL to use built in font
	 */
	virtual void setFont(const GFXfont* font) = 0;

	virtual GFXfont* getGfxFont() = 0;

	/**
	 * Set text colour with transparent background
	 */
	virtual void setTextColor(Colour c) = 0;
	/**
	 * Set text colour with custom background colour
	 */
	virtual void setTextColor(Colour c, Colour bg) = 0;

	virtual Colour getTextColor() const = 0;
	virtual Colour getTextBkgColor() const = 0;

	virtual void setCursor(int16_t x, int16_t y) = 0;
	virtual void print(const char* str) = 0;
	virtual void print(const __FlashStringHelper* str) = 0;

	/**
	 * @brief  Helper to determine size of a string with current font/size.
	 * 		  Pass string and a cursor position, returns UL corner and W,H.
	 * @param  str  The ASCII string to measure
	 * @param  x    The current cursor X
	 * @param  y    The current cursor Y
	 * @param  x1   The boundary X coordinate, returned by function
	 * @param  y1   The boundary Y coordinate, returned by function
	 * @param  w    The boundary width, returned by function
	 * @param  h    The boundary height, returned by function
	 */
	virtual void getTextBounds(const char *str, int16_t x, int16_t y,
									int16_t *x1, int16_t *y1, uint16_t *w,
									uint16_t *h) = 0;
	virtual void getTextBounds(const __FlashStringHelper *str, int16_t x, int16_t y,
									int16_t *x1, int16_t *y1, uint16_t *w,
									uint16_t *h) = 0;

	/**
	 * \param brightness 0 (off) to 100 (max)
	 */
	virtual void setBacklightBrightness(uint8_t brightness) = 0;

	/**
	 * Touch interface
	 */
	
};

} // namespace cz
