#pragma once

#include "crazygaze/micromuc/czmicromuc.h"
#include "crazygaze/micromuc/StringUtils.h"

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


// Assign human-readable names to some common 16-bit color values:
#define BLACK       0x0000
#define BLUE        0x001F
#define CYAN        0x07FF
#define DARKGREEN   0x03E0
#define DARKCYAN    0x03EF
#define DARKGREY    0x7BEF
#define GREEN       0x07E0
#define GREENYELLOW 0xB7E0
#define LIGHTGREY   0xC618
#define MAGENTA     0xF81F
#define MAROON      0x7800
#define NAVY        0x000F
#define OLIVE       0x7BE0
#define ORANGE      0xFDA0
#define PINK        0xFC9F
#define PURPLE      0x780F
#define RED         0xF800
#define WHITE       0xFFFF
#define YELLOW      0xFFE0

#define VERYDARKGREY    0x2945

namespace cz
{

extern MCUFRIEND_kbv gScreen;

struct Rect
{
	int16_t x;
	int16_t y;
	uint16_t width;
	uint16_t height;
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
void drawRGBBitmap(int16_t x, int16_t y, const uint16_t *bitmap, const uint8_t* mask, int16_t w, int16_t h, uint16_t bkgColor);
void drawRGBBitmapDisabled(int16_t x, int16_t y, const uint16_t *bitmap, const uint8_t* mask, int16_t w, int16_t h, uint16_t bkgColor);

/**
 * Prints a string aligned in a box area
 */
void printAligned(const Rect& area, HAlign halign, VAlign valign, const char* txt);
void printAligned(const Rect& area, HAlign halign, VAlign valign, const __FlashStringHelper* txt);


/**
 * 
 */
class MyButton : public Adafruit_GFX_Button
{
public:

	void drawButton(boolean inverted = false );
	
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
	uint16_t textColor;
	uint16_t bkgColor;
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
	uint16_t textColor;
	uint16_t bkgColor;
	unsigned int flags;
};

class BaseLabel : public Widget
{

protected:

	void drawImpl(const StaticLabelData& data)
	{
		if (data.flags & GFX_FLAG_ERASEBKG)
		{
			fillRect(data.pos, data.bkgColor);
		}

		if (data.flags & GFX_FLAG_DRAWBORDER)
		{
			drawRect(data.pos, data.textColor);
		}

		gScreen.setFont(data.font);
		gScreen.setTextColor(data.textColor);
		printAligned(data.pos, data.halign, data.valign, data.value);
	}

	void drawImpl(const FixedLabelData& data, const char* value)
	{
		if (data.flags & GFX_FLAG_ERASEBKG)
		{
			fillRect(data.pos, data.bkgColor);
		}

		if (data.flags & GFX_FLAG_DRAWBORDER)
		{
			drawRect(data.pos, data.textColor);
		}

		gScreen.setFont(data.font);
		gScreen.setTextColor(data.textColor);
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

