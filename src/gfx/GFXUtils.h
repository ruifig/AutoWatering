#pragma once

#include "crazygaze/micromuc/czmicromuc.h"
#include "crazygaze/micromuc/StringUtils.h"
#include "Colour.h"

#include <Adafruit_GFX.h>
#include <MCUFRIEND_kbv.h>

#include <FreeDefaultFonts.h>
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSans12pt7b.h>
#include <Fonts/Org_01.h>

#define TINY_FONT &Org_01
#define SMALL_FONT &FreeSmallFont
#define MEDIUM_FONT &FreeSans9pt7b
#define LARGE_FONT &FreeSans12pt7b

#define DEFAULT_FONT SMALL_FONT

namespace cz
{

extern MCUFRIEND_kbv gScreen;

struct Image
{
	const uint16_t* bmp;
	const uint8_t* mask;
	uint16_t width;
	uint16_t height;
};

struct Pos
{
	int16_t x;
	int16_t y;
};

struct Rect
{
	int16_t x;
	int16_t y;
	uint16_t width;
	uint16_t height;

	bool contains(int16_t x, int16_t y) const
	{
	  return ((x >= this->x) && (x < (int16_t)(this->x + this->width)) &&
	          (y >= this->y) && (y < (int16_t)(this->y + this->height)));
	}
};

enum class HAlign : uint8_t
{
	Left,
	Center,
	Right
};

enum class VAlign : uint8_t
{
	Top,
	Center,
	Bottom
};

void initializeScreen();

void fillRect(const Rect& box, uint16_t color);

void drawRect(const Rect& box, uint16_t color);

/**
 * Draws a filled rectangle with the specified colour, followed by a 565 RGB bitmap from PROGMEM using a bitmask
 * (set bits = opaque, unset bits = clear).
 **/
void drawRGBBitmap_P(int16_t x, int16_t y, const uint16_t *bitmap, const uint8_t* mask, int16_t w, int16_t h, uint16_t bkgColour);
void drawRGBBitmapDisabled_P(int16_t x, int16_t y, const uint16_t *bitmap, const uint8_t* mask, int16_t w, int16_t h, uint16_t bkgColour);
void drawRGBBitmap_P(const Rect& area, const uint16_t *bitmap, const uint8_t* mask, uint16_t bkgColour);
void drawRGBBitmapDisabled_P(const Rect& area, const uint16_t *bitmap, const uint8_t* mask, uint16_t bkgColour);

/**
 * Prints a string aligned in a box area
 */
void printAligned(const Rect& area, HAlign halign, VAlign valign, const char* txt);
void printAligned(const Rect& area, HAlign halign, VAlign valign, const __FlashStringHelper* txt);


/**
 * 
 */
class TextButton : public Adafruit_GFX_Button
{
public:

	void drawButton(boolean inverted = false );
};

enum class ButtonState : uint8_t
{
	Hidden,
	Disabled,
	Pressed,
	Released
};

/**
 * Image button, with the used bitmap being in PROGMEM
 */
class ImageButton
{
  public:
	ImageButton();
	void init(Adafruit_GFX &gfx, const Image& img, const Pos& pos, uint16_t bkgColour);
	void draw(bool forceDraw = false);
	bool contains(int16_t x, int16_t y) const;
	bool setEnabled(bool enabled);
	bool isDisabled() const
	{
		return m_currState == ButtonState::Disabled;
	}

	void setState(ButtonState state);
	bool justReleased() const;

  protected:
	Adafruit_GFX* m_gfx;
	Pos m_pos;
	Image m_img;
	uint16_t m_bkgColour;
	ButtonState m_lastState;
	ButtonState m_currState;
	bool m_needsRedraw;
};



namespace gfx
{

// If set, it fills the drawing area with the background colour
#define GFX_FLAG_ERASEBKG   (1 << 0)
// If set, it draw a rectangle with the foreground/text colour
#define GFX_FLAG_DRAWBORDER (1 << 1)

// For labels, it display the text as a percentage number. E.g: "90%"
#define GFX_FLAG_MUMASPERCENTAGE (1 << 15)


class Widget
{
  public:
	virtual void draw() = 0;
  private:
};


/**
 * Data for a label that doesn't change at all, and this all data can be in PROGMEM
 */
struct StaticLabelData
{
	Rect pos;
	HAlign halign;
	VAlign valign;
	const __FlashStringHelper* value;
	const GFXfont* font;
	Colour textColour;
	Colour bkgColour;
	unsigned int flags;
};

/**
 * Data for a label that the text can change, but not the other parameters
 */
struct FixedLabelData
{
	Rect pos;
	HAlign halign;
	VAlign valign;
	const GFXfont* font;
	Colour textColour;
	Colour bkgColour;
	unsigned int flags;
};

class BaseLabel : public Widget
{

protected:

	void drawImpl(const StaticLabelData& data)
	{
		if (data.flags & GFX_FLAG_ERASEBKG)
		{
			fillRect(data.pos, (uint16_t)data.bkgColour);
		}

		if (data.flags & GFX_FLAG_DRAWBORDER)
		{
			drawRect(data.pos, (uint16_t)data.textColour);
		}

		gScreen.setFont(data.font);
		gScreen.setTextColor((uint16_t)data.textColour);
		printAligned(data.pos, data.halign, data.valign, data.value);
	}

	void drawImpl(const FixedLabelData& data, const char* value)
	{
		if (data.flags & GFX_FLAG_ERASEBKG)
		{
			fillRect(data.pos, (uint16_t)data.bkgColour);
		}

		if (data.flags & GFX_FLAG_DRAWBORDER)
		{
			drawRect(data.pos, (uint16_t)data.textColour);
		}

		gScreen.setFont(data.font);
		gScreen.setTextColor((uint16_t)data.textColour);
		printAligned(data.pos, data.halign, data.valign, value);
	}

};

/**
 * Label where nothing can change.
 * The entire setup can be put in PROGMEM
 */
class StaticLabel : public BaseLabel
{
  public:
	StaticLabel(const StaticLabelData* data_P)
		: m_data_P(data_P)
	{
	}

	virtual void draw() override
	{
		StaticLabelData data;
		memcpy_P(&data, m_data_P, sizeof(data));
		drawImpl(data);
	}

  private:
	const StaticLabelData* m_data_P;
};


/**
 * Label that the text can change, but not the rest of the setup.
 */
template<int BUFSIZE=10>
class FixedLabel : public BaseLabel
{
  public:
	static int constexpr m_bufSize = BUFSIZE;

	FixedLabel(const FixedLabelData* data_P, const __FlashStringHelper* value = nullptr)
		: m_data_P(data_P)
	{
		if (value)
		{
			strncpy_P(m_value, (const char*)value, m_bufSize);
			m_value[m_bufSize-1] = 0;
		}
		else
		{
			strncpy_P(m_value, (const char*)F("99%"), m_bufSize);
			m_value[m_bufSize-1] = 0;
		}
	}

	virtual void draw() override
	{
		FixedLabelData data;
		memcpy_P(&data, m_data_P, sizeof(data));
		drawImpl(data, m_value);
	}

  private:
	const FixedLabelData* m_data_P;
	char m_value[m_bufSize];
};

class FixedNumLabel : public BaseLabel
{
  public:

	FixedNumLabel(const FixedLabelData* data_P, int value = 0)
		: m_data_P(data_P)
		, m_value(value)
		, m_needsRedraw(true)
	{
	}

	virtual void draw() override
	{
		if (!m_needsRedraw)
			return;

		FixedLabelData data;
		memcpy_P(&data, m_data_P, sizeof(data));
		const char *str;
		if (data.flags & GFX_FLAG_MUMASPERCENTAGE)
		{
			str = formatString(F("%3u%%"), m_value);
		}
		else
		{
			str = itoa(m_value, getTemporaryString(), 10);
		}

		drawImpl(data, str);
		m_needsRedraw = false;
	}

	void setValue(int value)
	{
		if (value != m_value)
		{
			m_value = value;
			m_needsRedraw = true;
		}
	}

	void setValueAndDraw(int value)
	{
		setValue(value);
		draw();
	}

  private:
	const FixedLabelData* m_data_P;
	int m_value;
	bool m_needsRedraw;
};



} // namespace gfx



} // namespace cz

