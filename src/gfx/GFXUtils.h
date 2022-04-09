#pragma once

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wignored-qualifiers"
	#include "Adafruit_GFX.h"
#pragma GCC diagnostic pop

#include "crazygaze/micromuc/czmicromuc.h"
#include "crazygaze/micromuc/StringUtils.h"
#include "Colour.h"

#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSans12pt7b.h>
#include <Fonts/Org_01.h>
#include "FreeDefaultFonts.h"

#define TINY_FONT &Org_01
#define SMALL_FONT &FreeSmallFont
#define MEDIUM_FONT &FreeSans9pt7b
#define LARGE_FONT &FreeSans12pt7b

#define DEFAULT_FONT SMALL_FONT

namespace cz
{

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

struct VLine
{
	Pos p;
	uint16_t height;

	constexpr int16_t top() const { return p.y; }
	constexpr int16_t bottom() const
	{
		return static_cast<int16_t>(p.y+height-1);
	}
};

struct HLine
{
	Pos p;
	uint16_t width;
	constexpr int16_t left() const { return p.x; }
	constexpr int16_t right() const
	{
		return static_cast<int16_t>(p.x+width-1);
	}
};

struct Rect
{
	int16_t x;
	int16_t y;
	uint16_t width;
	uint16_t height;

	Rect() = default;

	constexpr Rect(int16_t x, int16_t y, uint16_t width, uint16_t height)
		: x(x)
		, y(y)
		, width(width)
		, height(height)
	{
	}

	constexpr Rect(const Pos& pos, uint16_t width, uint16_t height)
		: x(pos.x)
		, y(pos.y)
		, width(width)
		, height(height)
	{
	}

	//
	// Initializes from two points. [topleft, bottomRight)
	constexpr Rect(const Pos& topLeft, const Pos& bottomRight)
		: x(topLeft.x)
		, y(topLeft.y)
		, width(bottomRight.x - topLeft.x)
		, height(bottomRight.y - topLeft.y)
	{
	}

	constexpr bool contains(int16_t x, int16_t y) const
	{
	  return ((x >= this->x) && (x < (int16_t)(this->x + this->width)) &&
	          (y >= this->y) && (y < (int16_t)(this->y + this->height)));
	}

	constexpr bool contains(const Pos& pos) const
	{
		return contains(pos.x, pos.y);
	}

	// Returns a Rect expanded the number of specified pixels in all 4 directions (top/bottom/left/right)
	constexpr Rect expand(int16_t pixels) const
	{
		return {
			static_cast<int16_t>(x - pixels),
			static_cast<int16_t>(y - pixels),
			static_cast<uint16_t>(width + pixels*2),
			static_cast<uint16_t>(height + pixels*2)
			};
	}

	constexpr int16_t left() const
	{
		return x;
	}
	constexpr int16_t right() const
	{
		return static_cast<int16_t>(x + width -1);
	}

	constexpr int16_t top() const
	{
		return y;
	}
	constexpr int16_t bottom() const
	{
		return static_cast<int16_t>(y + height -1);
	}

	constexpr Pos topLeft() const
	{
		return {x, y};
	}

	constexpr Pos topRight() const
	{
		return {right(), y};
	}

	constexpr Pos bottomLeft() const
	{
		return {x, bottom()};
	}

	constexpr Pos bottomRight() const
	{
		return { right(), bottom() };
	}

	constexpr VLine leftLine() const
	{
		return {topLeft(), height};
	}

	constexpr VLine rightLine() const
	{
		return {topRight(), height};
	}

	constexpr HLine topLine() const
	{
		return {topLeft(), width};
	}

	constexpr HLine bottomLine() const
	{
		return {bottomLeft(), width};
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

void fillRect(const Rect& box, Colour color);

void drawRect(const Rect& box, Colour color);

/**
 * Draws a filled rectangle with the specified colour, followed by a 565 RGB bitmap from PROGMEM using a bitmask
 * (set bits = opaque, unset bits = clear).
 **/
void drawRGBBitmap_P(int16_t x, int16_t y, const uint16_t *bitmap_P, const uint8_t* mask_P, int16_t w, int16_t h, Colour bkgColour);
void drawRGBBitmapDisabled_P(int16_t x, int16_t y, const uint16_t *bitmap_P, const uint8_t* mask_P, int16_t w, int16_t h, Colour bkgColour);
void drawRGBBitmap_P(const Rect& area, const uint16_t *bitmap_P, const uint8_t* mask_P, Colour bkgColour);
void drawRGBBitmapDisabled_P(const Rect& area, const uint16_t *bitmap_P, const uint8_t* mask_P, Colour bkgColour);

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


} // namespace cz

