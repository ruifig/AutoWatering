#pragma once

#include "crazygaze/micromuc/czmicromuc.h"
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

/**
 * Draws a filled rectangle with the specified colour, followed by a 565 RGB bitmap from PROGMEM using a bitmask
 * (set bits = opaque, unset bits = clear).
 **/
void drawRGBBitmap(int16_t x, int16_t y, const uint16_t *bitmap, const uint8_t* mask, int16_t w, int16_t h, uint16_t bkgColor);

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

#define GFX_FLAG_ERASEBKG 0x1
#define GFX_FLAG_DRAWBORDER 0x2


class Widget
{
  public:
	virtual void draw() = 0;
  private:
};

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

class StaticLabel : Widget
{
public:
	StaticLabel(StaticLabelData* data_P)
		: m_data_P(data_P)
	{
	}

	virtual void draw() override
	{
		StaticLabelData data;
		memcpy_P(&data, &m_data_P, sizeof(data));
		if (data.flags & GFX_FLAG_ERASEBKG)
		{
			fillRect(data.pos, data.bkgColor);
		}

		gScreen.setFont(data.font);
		gScreen.setTextColor(data.textColor);
		printAligned(data.pos, data.halign, data.valign, data.value);
	}

private:
	const StaticLabelData* m_data_P;
};


} // namespace gfx



} // namespace cz

