#include "TFTeSPIWrapper.h"
#include "crazygaze/micromuc/Logging.h"

namespace cz
{

TFTeSPIWrapper* TFTeSPIWrapper::ms_instance = nullptr;

#if AW_TOUCHUI_ENABLED
	TFTeSPIWrapper gScreen;
#endif

TFTeSPIWrapper::TFTeSPIWrapper()
	: m_bkpin(TFT_BL)
	, m_tft()
{
	CZ_ASSERT(ms_instance == nullptr);
	ms_instance = this;
}

TFTeSPIWrapper::~TFTeSPIWrapper()
{
	ms_instance = nullptr;
}

TFTeSPIWrapper* TFTeSPIWrapper::getInstance()
{
	return ms_instance;
}

bool TFTeSPIWrapper::begin()
{

	// Seems like I can't figure out how to reliably detect if a screen is attached.
	// This works in a Debug build but not Release. Not sure why, so disabling it this code for now
	#if 0
	m_tft.init();
	delay(150);
	// Reset.
	// This will make the screen go "white" but we can still read registers
	digitalWrite(TFT_RST, LOW);
	delay(10);
	digitalWrite(TFT_RST, HIGH);
	delay(10);
	uint32_t id = readRegister(ILI9341_RDDID, 3, 1);
	if (id != 0x00804E0F)
	{
		return false;
	}
	#endif

	delay(10);
	m_tft.init();
	//logProperties();

	m_tft.setRotation(3);
	pinMode(m_bkpin, OUTPUT);
	setBacklightBrightness(AW_SCREEN_DEFAULT_BRIGHTNESS);
	fillScreen(Colour_White);
	setTextColor(Colour_White);
	return true;
}

uint32_t TFTeSPIWrapper::readRegister(uint8_t reg, int16_t bytes, uint8_t index)
{
	uint32_t data = 0;

	while (bytes > 0)
	{
		bytes--;
		data = (data << 8) | m_tft.readcommand8(reg, index);
		index++;
	}

	CZ_LOG(logDefault, Log, "Display Register 0x%X: 0x%X", reg, data);

	return data;
}

void TFTeSPIWrapper::setBacklightBrightness(uint8_t brightness)
{
	analogWrite(m_bkpin, map(brightness, 0, 100, 0, 255));
}


/**
 * This doesn't seem to work correctly. I suspect it might be Adafruit's code not doing the right thing reading data from these commands
 */
void TFTeSPIWrapper::logProperties()
{
	digitalWrite(TFT_RST, LOW);
	delay(10);
	digitalWrite(TFT_RST, HIGH);
	delay(10);

	auto readRegister = [&](uint8_t reg, int16_t bytes, uint8_t index) -> uint32_t
	{
		uint32_t  data = 0;

		while (bytes > 0) {
			bytes--;
			data = (data << 8) | m_tft.readcommand8(reg, index);
			index++;
		}

		AW_CUSTOM_SERIAL.print("Register 0x");
		if (reg < 0x10) AW_CUSTOM_SERIAL.print("0");
		AW_CUSTOM_SERIAL.print(reg , HEX);

		AW_CUSTOM_SERIAL.print(": 0x");

		// Add leading zeros as needed
		uint32_t mask = 0x1 << 28;
		while (data < mask && mask > 0x1) {
			AW_CUSTOM_SERIAL.print("0");
			mask = mask >> 4;
		}

		AW_CUSTOM_SERIAL.println(data, HEX);

		return data;
	};

	auto printSubset = [&]()
	{
		AW_CUSTOM_SERIAL.println();  AW_CUSTOM_SERIAL.println();
		readRegister(ILI9341_RDDID, 3, 1);
		readRegister(ILI9341_RDDST, 4, 1);
		readRegister(ILI9341_RDMODE, 1, 1);
		readRegister(ILI9341_RDMADCTL, 1, 1);
		readRegister(ILI9341_RDPIXFMT, 1, 1);
		readRegister(ILI9341_RDSELFDIAG, 1, 1);
		readRegister(ILI9341_RAMRD, 3, 1);

		readRegister(ILI9341_RDID1, 1, 1);
		readRegister(ILI9341_RDID2, 1, 1);
		readRegister(ILI9341_RDID3, 1, 1);
		readRegister(ILI9341_RDIDX, 1, 1); // ?
		readRegister(ILI9341_RDID4, 3, 1);  // ID
	};

	printSubset();
}

} // namespace cz
