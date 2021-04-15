#include "DisplayTFT.h"

#include <FreeDefaultFonts.h>
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSans12pt7b.h>

#define YP A3  // must be an analog pin, use "An" notation!
#define XM A2  // must be an analog pin, use "An" notation!
#define YM 9   // can be a digital pin
#define XP 8   // can be a digital pin

#define TS_MINX 105
#define TS_MINY 66
#define TS_MAXX 915
#define TS_MAXY 892

// Assign human-readable names to some common 16-bit color values:
#define	BLACK   0x0000
#define	BLUE    0x001F
#define	RED     0xF800
#define	GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF

#define SMALL_FONT &FreeSmallFont
#define MEDIUM_FONT &FreeSans9pt7b
#define LARGE_FONT &FreeSans12pt7b

namespace cz
{

#if LOG_ENABLED
const char* DisplayTFT::ms_stateNames[3] =
{
	"Initializing",
	"Intro",
	"Overview"
};
#endif

DisplayTFT::DisplayTFT(Context& ctx)
	: m_ctx(ctx)
	// For better pressure precision, we need to know the resistance
	// between X+ and X- Use any multimeter to read it
	// For the one we're using, its 300 ohms across the X plate
	, m_ts(XP, YP, XM, YM, 300)
{
}

void DisplayTFT::begin()
{
	onEnterState();
}


float DisplayTFT::tick(float deltaSeconds)
{
	m_timeInState += deltaSeconds;
	m_timeSinceLastTouch += deltaSeconds;

	CZ_LOG_LN("DisplayTFT::%s: state=%s, timeInState = %d", __FUNCTION__, ms_stateNames[(int)m_state], (int)m_timeInState);

	switch(m_state)
	{
	case State::Initializing:
		changeToState(State::Intro);
		break;
	case State::Intro:
		if (m_timeInState >= INTRO_DURATION)
		{
			changeToState(State::Overview);
		}
		break;
	case State::Overview:
		// TODO : Fill me
		break;
	default:
		CZ_UNEXPECTED();
	}

#if 0
	if (m_timeSinceLastTouch >= DEFAULT_LCD_BACKLIGHT_TIMEOUT)
	{
		m_screenOff = true;
		m_tft.fillScreen(BLACK);
	}
#endif

	return 1.0f / 5.0f;
}
	

void DisplayTFT::changeToState(State newState)
{
    CZ_LOG_LN("DisplayTFT::%s %dms %s->%s"
        , __FUNCTION__
		, (int)(m_timeInState * 1000.f)
        , ms_stateNames[(int)m_state]
        , ms_stateNames[(int)newState]);

    onLeaveState();
	m_tft.fillScreen(BLACK);
    m_state = newState;
    m_timeInState = 0.0f;
    onEnterState();
}

void DisplayTFT::onLeaveState()
{
	switch (m_state)
	{
	case State::Initializing:
		break;
	case State::Intro:
		break;
	case State::Overview:
		break;

	default:
		CZ_UNEXPECTED();
	}
}

void DisplayTFT::onEnterState()
{
	switch(m_state)
	{
	case State::Initializing:
		{
			m_tft.reset();
			uint16_t identifier = m_tft.readID();
			if(identifier == 0x9325)
			{
				Serial.println(F("Found ILI9325 LCD driver"));
				} else if(identifier == 0x9328) {
				Serial.println(F("Found ILI9328 LCD driver"));
				} else if(identifier == 0x7575) {
				Serial.println(F("Found HX8347G LCD driver"));
				} else if(identifier == 0x9341) {
				Serial.println(F("Found ILI9341 LCD driver"));
				} else if(identifier == 0x8357) {
				Serial.println(F("Found HX8357D LCD driver"));
				} else {
				Serial.print(F("Unknown LCD driver chip: "));
				Serial.println(identifier, HEX);
				Serial.println(F("If using the Adafruit 2.8\" TFT Arduino shield, the line:"));
				Serial.println(F("  #define USE_ADAFRUIT_SHIELD_PINOUT"));
				Serial.println(F("should appear in the library header (Adafruit_TFT.h)."));
				Serial.println(F("If using the breakout board, it should NOT be #defined!"));
				Serial.println(F("Also if using the breakout, double-check that all wiring"));
				Serial.println(F("matches the tutorial."));
				return;
			}

			m_tft.begin(identifier);
			m_tft.fillScreen(BLACK);
			m_tft.setRotation(1); // LANDSCAPE
		}
		break;

	case State::Intro:
		m_tft.setTextColor(RED);
		m_tft.setCursor(0,20);

		m_tft.setFont(SMALL_FONT);
		m_tft.print("Small font! ");

		m_tft.setFont(MEDIUM_FONT);
		m_tft.print("Medium font! ");

		m_tft.setFont(LARGE_FONT);
		m_tft.print("Large font! ");

		break;

	case State::Overview:
		break;

	default:
		CZ_UNEXPECTED();
	}
}

} // namespace cz
