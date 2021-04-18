#include "DisplayTFT.h"
#include "Utils.h"
#include "crazygaze/micromuc/Logging.h"

#include <FreeDefaultFonts.h>
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSans12pt7b.h>
#include <Fonts/Org_01.h>



#define YP A3  // must be an analog pin, use "An" notation!
#define XM A2  // must be an analog pin, use "An" notation!
#define YM 9   // can be a digital pin
#define XP 8   // can be a digital pin

#define TS_MINX 105
#define TS_MINY 66
#define TS_MAXX 915
#define TS_MAXY 892

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


#define TINY_FONT &Org_01
#define SMALL_FONT &FreeSmallFont
#define MEDIUM_FONT &FreeSans9pt7b
#define LARGE_FONT &FreeSans12pt7b

namespace cz
{

#if CZ_LOG_ENABLED
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

	CZ_LOG(logDefault, Log, "DisplayTFT::%s: state=%s, timeInState = %d", __FUNCTION__, ms_stateNames[(int)m_state], (int)m_timeInState);

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
    CZ_LOG(logDefault, Log, "DisplayTFT::%s %dms %s->%s"
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


void DisplayTFT::printAligned(const Box& area, HAlign halign, VAlign valign, const char* txt)
{
	Box bounds;
	m_tft.getTextBounds(txt, 0,0, &bounds.x, &bounds.y, &bounds.width, &bounds.height);

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

	//m_tft.drawRect(area.x, area.y, area.width, area.height, BLUE);

	m_tft.setCursor(x,y);
	m_tft.print(txt);
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
				CZ_LOG(logDefault, Log, "Found ILI9325 LCD driver");
			} else if(identifier == 0x9328) {
				CZ_LOG(logDefault, Log, "Found ILI9328 LCD driver");
			} else if(identifier == 0x7575) {
				CZ_LOG(logDefault, Log, "Found HX8347G LCD driver");
			} else if(identifier == 0x9341) {
				CZ_LOG(logDefault, Log, "Found ILI9341 LCD driver");
			} else if(identifier == 0x8357) {
				CZ_LOG(logDefault, Log, "Found HX8357D LCD driver");
			} else {
				CZ_LOG(logDefault, Log, "Unknown LCD driver chip: 0x%x", identifier);
				CZ_LOG(logDefault, Log, "If using the Adafruit 2.8\" TFT Arduino shield, the line:");
				CZ_LOG(logDefault, Log, "  #define USE_ADAFRUIT_SHIELD_PINOUT");
				CZ_LOG(logDefault, Log, "should appear in the library header (Adafruit_TFT.h).");
				CZ_LOG(logDefault, Log, "If using the breakout board, it should NOT be #defined!");
				CZ_LOG(logDefault, Log, "Also if using the breakout, double-check that all wiring");
				CZ_LOG(logDefault, Log, "matches the tutorial.");
				return;
			}

			m_tft.begin(identifier);
			m_tft.fillScreen(BLACK);
			m_tft.setRotation(1); // LANDSCAPE
		}
		break;

	case State::Intro:
		m_tft.setTextColor(GREEN);
		m_tft.setCursor(0,20);
		
		m_tft.setFont(LARGE_FONT);

		printAligned({10, 10, m_tft.width()-20, m_tft.height()-20}, HAlign::Left, VAlign::Top, "AutoWatering");
		printAligned({10, 10, m_tft.width()-20, m_tft.height()-20}, HAlign::Center, VAlign::Center, "By");
		printAligned({10, 10, m_tft.width()-20, m_tft.height()-20}, HAlign::Right, VAlign::Bottom, "Rui Figueira");

		break;

	case State::Overview:
		break;

	default:
		CZ_UNEXPECTED();
	}
}

} // namespace cz
