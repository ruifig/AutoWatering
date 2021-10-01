#include "GFXUtils.h"
#include "crazygaze/micromuc/Logging.h"

namespace cz
{

MCUFRIEND_kbv gScreen;

void initializeScreen()
{
	gScreen.reset();
	uint16_t identifier = gScreen.readID();
	if(identifier == 0x9325)
	{
		CZ_LOG(logDefault, Log, F("Found ILI9325 LCD driver"));
	} else if(identifier == 0x9328) {
		CZ_LOG(logDefault, Log, F("Found ILI9328 LCD driver"));
	} else if(identifier == 0x7575) {
		CZ_LOG(logDefault, Log, F("Found HX8347G LCD driver"));
	} else if(identifier == 0x9341) {
		CZ_LOG(logDefault, Log, F("Found ILI9341 LCD driver"));
	} else if(identifier == 0x8357) {
		CZ_LOG(logDefault, Log, F("Found HX8357D LCD driver"));
	} else {
		CZ_LOG(logDefault, Log, F("Unknown LCD driver chip: 0x%x"), identifier);
		CZ_LOG(logDefault, Log, F("If using the Adafruit 2.8\" TFT Arduino shield, the line:"));
		CZ_LOG(logDefault, Log, F("  #define USE_ADAFRUIT_SHIELD_PINOUT"));
		CZ_LOG(logDefault, Log, F("should appear in the library header (Adafruit_TFT.h)."));
		CZ_LOG(logDefault, Log, F("If using the breakout board, it should NOT be #defined!"));
		CZ_LOG(logDefault, Log, F("Also if using the breakout, double-check that all wiring"));
		CZ_LOG(logDefault, Log, F("matches the tutorial."));
		return;
	}

	gScreen.begin(identifier);
	gScreen.fillScreen(Colour_Black);
	gScreen.setRotation(1); // LANDSCAPE
}

//////////////////////////////////////////////////////////////////////////
// Utility drawing
//////////////////////////////////////////////////////////////////////////

void fillRect(const Rect& box, uint16_t color)
{
	gScreen.fillRect(box.x, box.y, box.width, box.height, color);
}

void drawRect(const Rect& box, uint16_t color)
{
	gScreen.drawRect(box.x, box.y, box.width, box.height, color);
}

void drawRGBBitmap_P(int16_t x, int16_t y, const uint16_t *bitmap_P, const uint8_t* mask_P, int16_t w, int16_t h, uint16_t bkgColour)
{
	int16_t bw = (w + 7) / 8; // Bitmask scanline pad = whole byte
	uint8_t byte = 0;
	gScreen.startWrite();
	for (int16_t j = 0; j < h; j++, y++) {
		for (int16_t i = 0; i < w; i++) {
			if (i & 7)
				byte <<= 1;
			else
				byte = pgm_read_byte(&mask_P[j * bw + i / 8]);
			if (byte & 0x80) {
				gScreen.writePixel(x + i, y, pgm_read_word(&bitmap_P[j * w + i]));
			}
			else
			{
				gScreen.writePixel(x + i, y, bkgColour);
			}
		}
	}
	gScreen.endWrite();
}

void drawRGBBitmap_P(const Rect& area, const uint16_t *bitmap, const uint8_t* mask, uint16_t bkgColour)
{
	drawRGBBitmap_P(area.x, area.y, bitmap, mask, area.width, area.height, bkgColour);
}

void drawRGBBitmapDisabled_P(int16_t x, int16_t y, const uint16_t *bitmap_P, const uint8_t* mask_P, int16_t w, int16_t h, uint16_t bkgColour)
{
	int count = 0;
	int16_t bw = (w + 7) / 8; // Bitmask scanline pad = whole byte
	uint8_t byte = 0;
	gScreen.startWrite();
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
					gScreen.writePixel(x + i, y, Colour_Black);
				else
					gScreen.writePixel(x + i, y, Colour(pgm_read_word(&bitmap_P[j * w + i])).toGrey());
			}
			else
			{
				gScreen.writePixel(x + i, y, bkgColour);
			}
		}
	}
	gScreen.endWrite();
}

void drawRGBBitmapDisabled_P(const Rect& area, const uint16_t *bitmap_P, const uint8_t* mask_P, uint16_t bkgColour)
{
	drawRGBBitmapDisabled_P(area.x, area.y, bitmap_P, mask_P, area.width, area.height, bkgColour);
}

template<typename T>
void printAlignedImpl(const Rect& area, HAlign halign, VAlign valign, const T* txt)
{
	gScreen.setTextSize(0,0);
	Rect bounds;
	gScreen.getTextBounds(txt, 0,0, &bounds.x, &bounds.y, &bounds.width, &bounds.height);


	int x = area.x;
	int y = area.y;
	
	switch(halign)
	{
	case HAlign::Left:
		x = area.x - bounds.x/2;
		break;
	case HAlign::Center:
		x = (area.x + area.width/2) - (bounds.width/2) - (bounds.x/2);
		break;
	case HAlign::Right:
		x = area.x + area.width - bounds.width - bounds.x;
		break;
	}

	switch(valign)
	{
	case VAlign::Top:
		y = area.y - bounds.y;
		break;
	case VAlign::Center:
		y = (area.y + area.height/2) - (bounds.height/2) - bounds.y;
		break;
	case VAlign::Bottom:
		y = area.y + area.height - (bounds.height + bounds.y);
		break;
	}

	gScreen.setCursor(x,y);
	gScreen.print(txt);
}

void printAligned(const Rect& area, HAlign halign, VAlign valign, const char* txt)
{
	printAlignedImpl(area, halign, valign, txt);
}

void printAligned(const Rect& area, HAlign halign, VAlign valign, const __FlashStringHelper* txt)
{
	printAlignedImpl(area, halign, valign, txt);
}






} // namespace cz

