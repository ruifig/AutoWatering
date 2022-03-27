#pragma once

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wignored-qualifiers"
	#include "Adafruit_GFX.h"
#pragma GCC diagnostic pop

#include "Colour.h"

namespace cz
{

class GraphicsInterface
{
  public:

	virtual int16_t height() const = 0;
	virtual int16_t width() const = 0;

	virtual void fillScreen(Colour colour) = 0;
	virtual void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, Colour color) = 0;
	virtual void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, Colour color) = 0;


	//
	// Text rendering
	//

	/**
	 * \param font Font to use, or NULL to use built in font
	 */
	virtual void setFont(const GFXfont* font) = 0;

	/**
	 * Set text colour with transparent background
	 */
	virtual void setTextColor(Colour c) = 0;
	/**
	 * Set text colour with custom background colour
	 */
	virtual void setTextColor(Colour c, Colour bg) = 0;
	virtual void setCursor(int16_t x, int16_t y) = 0;
	virtual void print(const char* str) = 0;

	/**
	 * \param brightness 0 (off) to 255 (max)
	 */
	virtual void setBacklightBrightness(uint8_t brightness) = 0;

	/**
	 * Touch interface
	 */
	
};

} // namespace cz
