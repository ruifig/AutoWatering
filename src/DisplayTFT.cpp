#include "DisplayTFT.h"
#include "Utils.h"
#include "crazygaze/micromuc/Logging.h"
#include "crazygaze/micromuc/StringUtils.h"
#include <crazygaze/micromuc/Profiler.h>

#include <FreeDefaultFonts.h>
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSans12pt7b.h>
#include <Fonts/Org_01.h>

//
// chart_curve, 16*16
//
constexpr int16_t chart_curve_width = 16;
constexpr int16_t chart_curve_height = 16;
const uint16_t chart_curve[] PROGMEM = {
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 
	0x0000, 0x33b8, 0x5cbb, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 
	0x0000, 0x33b8, 0x3bf8, 0x4c9b, 0x0000, 0x0000, 0x0000, 0xeacb, 0xeaab, 0xeaab, 0xeaab, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 
	0x0000, 0x33b8, 0x343c, 0x3c5b, 0x3c7c, 0x0000, 0xf269, 0xf26a, 0xf26a, 0xea8a, 0xea8a, 0xf28a, 0xf28a, 0x0000, 0x0000, 0x0000, 
	0x0000, 0x33b8, 0x055f, 0x2bfc, 0x2c1c, 0xf1e8, 0xf208, 0xf228, 0x0000, 0x0000, 0xf209, 0xf229, 0xea49, 0x4c9b, 0x0000, 0x0000, 
	0x0000, 0x3398, 0x2230, 0x1bbc, 0x62f5, 0xf186, 0xf1a7, 0x0000, 0x0000, 0x0000, 0x33fc, 0x341b, 0x3c1b, 0x3c3b, 0x3c3b, 0x0000, 
	0x0000, 0x3398, 0x0000, 0x0000, 0xe947, 0xe188, 0x0000, 0x0000, 0x0000, 0x1b7c, 0x239b, 0x239b, 0xf986, 0xf186, 0xa2d1, 0x0000, 
	0x0000, 0x3398, 0x0000, 0xf8a3, 0xf8c3, 0x3a77, 0x0b1b, 0x02bf, 0x131b, 0x131b, 0x131b, 0x0000, 0x0000, 0xf945, 0xf145, 0x0000, 
	0x0000, 0x3397, 0x798a, 0xf862, 0xf862, 0x02dc, 0x02bc, 0x02bb, 0x02bb, 0x02bb, 0x0000, 0x0000, 0x0000, 0x0000, 0xf904, 0x0000, 
	0x0000, 0x3377, 0xf800, 0xf800, 0x0000, 0x0000, 0x0000, 0x027b, 0x4d08, 0x4d07, 0x4d27, 0x4526, 0x0000, 0x0000, 0xf8a3, 0x0000, 
	0x0000, 0x2b77, 0xf800, 0xf800, 0x0000, 0x0000, 0x4ca7, 0x4cc7, 0x44c6, 0x44c7, 0x44e6, 0x44e6, 0x4506, 0x0000, 0xf842, 0x0000, 
	0x0000, 0x2b77, 0x9884, 0x0000, 0x0000, 0x4426, 0x4446, 0x4446, 0x3ca5, 0x0000, 0x0000, 0x3c66, 0x3c85, 0x0000, 0x0000, 0x0000, 
	0x0000, 0x2b56, 0x3b85, 0x3ba5, 0x33a5, 0x33a5, 0x3ba5, 0x0000, 0x0000, 0x0000, 0x0000, 0x3be3, 0x3c05, 0x3405, 0x0000, 0x0000, 
	0x0000, 0x2b56, 0x3325, 0x2b45, 0x2a86, 0x3b83, 0x0000, 0x1148, 0x0000, 0x0000, 0x1108, 0x0000, 0x3344, 0x22c5, 0x2b45, 0x0000, 
	0x0000, 0x2b56, 0x2b56, 0x2b56, 0x2b56, 0x2b36, 0x2b35, 0x2b35, 0x2b35, 0x2b35, 0x2b35, 0x2b35, 0x2b35, 0x2b15, 0x2b15, 0x0000, 
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 
	
};
const uint8_t chart_curve_bitmask[] PROGMEM = {
	0x00, 0x00, 0x60, 0x00, 0x71, 0xe0, 0x7b, 0xf8, 
	0x7f, 0x3c, 0x7e, 0x3e, 0x4c, 0x7e, 0x5f, 0xe6, 
	0x7f, 0xc2, 0x71, 0xf2, 0x73, 0xfa, 0x67, 0x98, 
	0x7e, 0x1c, 0x7d, 0x2e, 0x7f, 0xfe, 0x00, 0x00, 
	
};

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

#define VERYDARKGREY    0x2945

#define TINY_FONT &Org_01
#define SMALL_FONT &FreeSmallFont
#define MEDIUM_FONT &FreeSans9pt7b
#define LARGE_FONT &FreeSans12pt7b

namespace cz
{

const char* const DisplayTFT::ms_stateNames[3]=
{
	"Initializing",
	"Intro",
	"Overview"
};

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
	PROFILE_SCOPE(F("DisplayTFT"));

	m_timeInState += deltaSeconds;
	m_timeSinceLastTouch += deltaSeconds;

	//CZ_LOG(logDefault, Log, F("DisplayTFT::%s: state=%s, timeInState = %d"), __FUNCTION__, ms_stateNames[(int)m_state], (int)m_timeInState);

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
		drawOverview();
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
    CZ_LOG(logDefault, Log, F("DisplayTFT::%s %dms %s->%s")
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


template<typename T>
void DisplayTFT::printAlignedImpl(const Box& area, HAlign halign, VAlign valign, const T* txt)
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

void DisplayTFT::drawRGBBitmap(int16_t x, int16_t y, const uint16_t *bitmap, const uint8_t* mask, int16_t w, int16_t h, uint16_t bkgColor)
{
	m_tft.fillRect(x, y, w, h, bkgColor);
	m_tft.drawRGBBitmap(x, y, bitmap, mask, w, h);
}


void DisplayTFT::fillRect(const Box& box, uint16_t color)
{
	m_tft.fillRect(box.x, box.y, box.width, box.height, color);
}

void DisplayTFT::plotHistory(int16_t x, int16_t y, int16_t h, const TFixedCapacityQueue<GraphPoint>& data, uint8_t valThreshold /*, const GraphPoint* oldData, int oldCount*/)
{
	int bottomY = y + h - 1;

	const int count = data.size();
	for(int i=0; i<count; i++)
	{
		GraphPoint p = data.getAtIndex(i);
		int xx = x + i;
		m_tft.drawFastVLine(xx, y, h, BLACK);
		// The height for the plotting is h-1 because we reserve the top pixel for the motor on/off
		//int yy = map(p.val, 0, 100, 0,  h - 2);
		int yy = p.val;
		m_tft.drawPixel(xx, bottomY - yy, p.val < valThreshold ? GRAPH_MOISTURE_LOW_COLOUR : GRAPH_MOISTURE_OK_COLOUR);
		m_tft.drawPixel(xx, y, p.on ? GRAPH_MOTOR_ON_COLOUR : GRAPH_MOTOR_OFF_COLOUR);
	}

}


void DisplayTFT::onEnterState()
{
	switch(m_state)
	{
	case State::Initializing:
		{
			memset(m_previousValues, 0, sizeof(m_previousValues));
			m_tft.reset();
			uint16_t identifier = m_tft.readID();
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

			m_tft.begin(identifier);
			m_tft.fillScreen(BLACK);
			m_tft.setRotation(1); // LANDSCAPE
		}
		break;

	case State::Intro:
		m_tft.setTextColor(GREEN);
		m_tft.setCursor(0,20);
		
		m_tft.setFont(LARGE_FONT);

		printAligned({10, 10, m_tft.width()-20, m_tft.height()-20}, HAlign::Left, VAlign::Top, F("AutoWatering"));
		printAligned({10, 10, m_tft.width()-20, m_tft.height()-20}, HAlign::Center, VAlign::Center, F("By"));
		printAligned({10, 10, m_tft.width()-20, m_tft.height()-20}, HAlign::Right, VAlign::Bottom, F("Rui Figueira"));


		m_tft.drawRGBBitmap(50,50, chart_curve, chart_curve_width, chart_curve_width);

		drawRGBBitmap(50,70, chart_curve, chart_curve_bitmask, chart_curve_width, chart_curve_height, WHITE);
		drawRGBBitmap(50,90, chart_curve, chart_curve_bitmask, chart_curve_width, chart_curve_height, YELLOW);

		break;

	case State::Overview:
		drawHistoryBoxes();
		break;

	default:
		CZ_UNEXPECTED();
	}
}


void DisplayTFT::drawHistoryBoxes()
{
	PROFILE_SCOPE(F("drawHistoryBoxes"));

	for(int i=0; i<NUM_MOISTURESENSORS; i++)
	{
		int x = m_historyX;
		int y = m_groupsStartY + (i*(GRAPH_HEIGHT + m_spaceBetweenGroups));
		m_tft.drawRect(x-1, y-1, GRAPH_NUMPOINTS+2, GRAPH_HEIGHT+2, VERYDARKGREY);

		constexpr int16_t h = GRAPH_HEIGHT;
		int bottomY = y + h - 1;
		m_tft.drawFastHLine(x-3, bottomY - map(0, 0, 100, 0, h - 1), 4, DARKGREY);
		m_tft.drawFastHLine(x-3, bottomY - map(20, 0, 100, 0, h - 1), 4, DARKGREY);
		m_tft.drawFastHLine(x-3, bottomY - map(40, 0, 100, 0, h - 1), 4, DARKGREY);
		m_tft.drawFastHLine(x-3, bottomY - map(60, 0, 100, 0, h - 1), 4, DARKGREY);
		m_tft.drawFastHLine(x-3, bottomY - map(80, 0, 100, 0, h - 1), 4, DARKGREY);
		m_tft.drawFastHLine(x-3, bottomY - map(100, 0, 100, 0, h - 1), 4, DARKGREY);
	}
}

void DisplayTFT::drawOverview()
{
	PROFILE_SCOPE(F("DisplayTFT:drawOverview"));

	for(int i=0; i<NUM_MOISTURESENSORS; i++)
	{
		PROFILE_SCOPE(F("sensorDrawing"));

		GroupData& data = m_ctx.data.getGroupData(i);
		const HistoryQueue& history = data.getHistory();

		int y = m_groupsStartY + (i*(GRAPH_HEIGHT + m_spaceBetweenGroups));
		//
		// Draw history
		//
		{
			PROFILE_SCOPE(F("plotHistory"));

			int x = m_historyX;
			plotHistory(x, y, GRAPH_HEIGHT, history, data.getPercentageThreshold());
		}

		//
		// Draw values
		//
		{
			PROFILE_SCOPE(F("drawValue"));

			PreviousValues& previousValues = m_previousValues[i];
			Box box = {m_historyX + GRAPH_NUMPOINTS + 2, y, 30, GRAPH_HEIGHT/3};
			m_tft.setFont(TINY_FONT);
			m_tft.setTextColor(DARKGREY);
			char str[5];

			if (previousValues.waterValue != data.getWaterValue())
			{
				previousValues.waterValue = data.getWaterValue();
				itoa(previousValues.waterValue, str, 10);
				fillRect(box, BLACK);
				printAligned(box, HAlign::Center, VAlign::Center, str);
			}

			box.y += box.height;
			if (previousValues.percentage != data.getPercentageValue())
			{
				previousValues.percentage = data.getPercentageValue();
				itoa(previousValues.percentage, str, 10);
				fillRect(box, BLACK);
				printAligned(box, HAlign::Center, VAlign::Center, formatString(F("%3u%%"), previousValues.percentage));
			}


			box.y += box.height;
			if (previousValues.airValue != data.getAirValue())
			{
				previousValues.airValue = data.getAirValue();
				itoa(data.getAirValue(), str, 10);
				fillRect(box, BLACK);
				printAligned(box, HAlign::Center, VAlign::Center, str);
			}

		}

	}

	

}

} // namespace cz
