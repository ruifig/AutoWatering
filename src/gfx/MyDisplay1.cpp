#include "MyDisplay1.h"
#include "crazygaze/micromuc/Logging.h"

namespace cz
{

MyDisplay1::MyDisplay1( MCUPin displayCS, MCUPin displayDC, MCUPin displayBacklight)
	: m_bkpin(displayBacklight)
	, m_tft(displayCS.raw, displayDC.raw)
{
}

void MyDisplay1::begin()
{
	// Which begin to use?
	m_tft.begin(42000000);
	//gTft.begin(30000000); // Stayed running for a very long time

	pinMode(m_bkpin, OUTPUT);
	// Set backlight to low brightness to save power by default
	setBacklightBrightness(40);

	fillScreen(Colour_Green);
	m_tft.setRotation(1);

	logProperties();
}

void MyDisplay1::setBacklightBrightness(uint8_t brightness)
{
	analogWrite(m_bkpin, brightness);
}

/**
 * This doesn't seem to work correctly. I suspect it might be Adafruit's code not doing the right thing reading data from these commands
 */
void MyDisplay1::logProperties()
{
	// read diagnostics (optional but can help debug problems)
	CZ_LOG(logDefault, Log, "Display properties (possibly not working as it should):")
	int x = m_tft.readcommand8(ILI9341_RDMODE);
	CZ_LOG(logDefault, Log, "Display Power Mode: 0x%x", x);
	x = m_tft.readcommand8(ILI9341_RDMADCTL);
	CZ_LOG(logDefault, Log, "MADCTL Mode: 0x%x", x);
	x = m_tft.readcommand8(ILI9341_RDPIXFMT);
	CZ_LOG(logDefault, Log, "Pixel Format: 0x%x", x);
	x = m_tft.readcommand8(ILI9341_RDIMGFMT);
	CZ_LOG(logDefault, Log, "Image Format: 0x%x", x);
	x = m_tft.readcommand8(ILI9341_RDSELFDIAG);
	CZ_LOG(logDefault, Log, "Self Diagnostic: 0x%x", x);
}

} // namespace cz
