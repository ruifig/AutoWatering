#include "GFXUtils.h"
#include "crazygaze/micromuc/Logging.h"
#include "TFTeSPIWrapper.h"

namespace cz
{

//////////////////////////////////////////////////////////////////////////
// Utility drawing
//////////////////////////////////////////////////////////////////////////

void fillRect(const Rect& box, Colour color)
{
	TFTeSPIWrapper::getInstance()->fillRect(box.x, box.y, box.width, box.height, color);
}

void drawRect(const Rect& box, Colour color)
{
	TFTeSPIWrapper::getInstance()->drawRect(box.x, box.y, box.width, box.height, color);
}

void drawRGBBitmap_P(int16_t x, int16_t y, const uint16_t *bitmap_P, const uint8_t* mask_P, int16_t w, int16_t h, Colour bkgColour)
{
	int16_t bw = (w + 7) / 8; // Bitmask scanline pad = whole byte
	uint8_t byte = 0;
	TFTeSPIWrapper::getInstance()->startWrite();
	for (int16_t j = 0; j < h; j++, y++) {
		for (int16_t i = 0; i < w; i++) {
			if (i & 7)
				byte <<= 1;
			else
				byte = pgm_read_byte(&mask_P[j * bw + i / 8]);
			if (byte & 0x80) {
				TFTeSPIWrapper::getInstance()->writePixel(x + i, y, Colour(pgm_read_word(&bitmap_P[j * w + i])));
			}
			else
			{
				TFTeSPIWrapper::getInstance()->writePixel(x + i, y, bkgColour);
			}
		}
	}
	TFTeSPIWrapper::getInstance()->endWrite();
}

void drawRGBBitmap_P(const Rect& area, const uint16_t *bitmap, const uint8_t* mask, Colour bkgColour)
{
	drawRGBBitmap_P(area.x, area.y, bitmap, mask, area.width, area.height, bkgColour);
}

void drawRGBBitmapDisabled_P(int16_t x, int16_t y, const uint16_t *bitmap_P, const uint8_t* mask_P, int16_t w, int16_t h, Colour bkgColour)
{
	int count = 0;
	int16_t bw = (w + 7) / 8; // Bitmask scanline pad = whole byte
	uint8_t byte = 0;
	TFTeSPIWrapper::getInstance()->startWrite();
	for (int16_t j = 0; j < h; j++, y++)
	{
		for (int16_t i = 0; i < w; i++)
		{
			count++;

			if (i & 7)
				byte <<= 1;
			else
				byte = pgm_read_byte(&mask_P[j * bw + i / 8]);

			if (byte & 0x80)
			{
				if ((count % 2) == 0)
					TFTeSPIWrapper::getInstance()->writePixel(x + i, y, Colour_Black);
				else
					TFTeSPIWrapper::getInstance()->writePixel(x + i, y, Colour(pgm_read_word(&bitmap_P[j * w + i])).toGrey());
			}
			else
			{
				TFTeSPIWrapper::getInstance()->writePixel(x + i, y, bkgColour);
			}
		}
	}
	TFTeSPIWrapper::getInstance()->endWrite();
}

void drawRGBBitmapDisabled_P(const Rect& area, const uint16_t *bitmap_P, const uint8_t* mask_P, Colour bkgColour)
{
	drawRGBBitmapDisabled_P(area.x, area.y, bitmap_P, mask_P, area.width, area.height, bkgColour);
}

template<typename T>
void printAlignedImpl(const Rect& area, HAlign halign, VAlign valign, const T* txt, bool eraseBackground)
{
	if (eraseBackground)
	{
		fillRect(area, TFTeSPIWrapper::getInstance()->getTextBkgColor());
	}

	Pos p;
	int datum;
	switch(valign)
	{
		case VAlign::Top:
			p.y = area.y;
			datum = 0;
			break;
		case VAlign::Center:
			p.y = area.y + area.height/2;
			datum = 1;
			break;
		case VAlign::Bottom:
			p.y = area.y + area.height-1;
			datum = 2;
			break;
	}

	datum *= 3;

	switch(halign)
	{
		case HAlign::Left:
			p.x = area.x;
			datum += 0;
			break;
		case HAlign::Center:
			p.x = area.x + area.width/2;
			datum += 1;
			break;
		case HAlign::Right:
			p.x = area.x + area.width-1;
			datum += 2;
			break;
	}

	TFTeSPIWrapper::getInstance()->getTFT_eSPI().setTextDatum(datum);
	TFTeSPIWrapper::getInstance()->getTFT_eSPI().drawString(txt, p.x, p.y);
}

void printAligned(const Rect& area, HAlign halign, VAlign valign, const char* txt, bool eraseBackground)
{
	printAlignedImpl(area, halign, valign, txt, eraseBackground);
}

void printAligned(const Rect& area, HAlign halign, VAlign valign, const __FlashStringHelper* txt, bool eraseBackground)
{
	printAlignedImpl(area, halign, valign, txt, eraseBackground);
}

} // namespace cz

