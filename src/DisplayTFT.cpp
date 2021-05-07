#include "DisplayTFT.h"
#include "Utils.h"
#include "gfx/GFXUtils.h"
#include "crazygaze/micromuc/Logging.h"
#include "crazygaze/micromuc/StringUtils.h"
#include <crazygaze/micromuc/Profiler.h>


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

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240

namespace cz
{

using namespace gfx;

namespace
{

constexpr static int16_t ms_historyX = 5;
constexpr static int16_t ms_groupsStartY = 10;
constexpr static int16_t ms_spaceBetweenGroups = 20;


const char introLabel1_value_P[] PROGMEM = "AutoWatering";
const StaticLabelData introLabel1_P PROGMEM =
{
	{10, 10, SCREEN_WIDTH - 20, SCREEN_HEIGHT - 20 }, // box
	HAlign::Left, VAlign::Top,  (const __FlashStringHelper*)introLabel1_value_P,
	LARGE_FONT,
	GREEN,
	BLACK,
	GFX_FLAG_ERASEBKG
};

const char introLabel2_value_P[] PROGMEM = "By";
const StaticLabelData introLabel2_P PROGMEM =
{
	{10, 10, SCREEN_WIDTH - 20, SCREEN_HEIGHT - 20 }, // box
	HAlign::Center, VAlign::Center,  (const __FlashStringHelper*)introLabel2_value_P,
	LARGE_FONT,
	GREEN,
	BLACK,
	0
};

const char introLabel3_value_P[] PROGMEM = "Rui Figueira";
const StaticLabelData introLabel3_P PROGMEM =
{
	{10, 10, SCREEN_WIDTH - 20, SCREEN_HEIGHT - 20 }, // box
	HAlign::Right, VAlign::Bottom,  (const __FlashStringHelper*)introLabel3_value_P,
	LARGE_FONT,
	GREEN,
	BLACK,
	0
};


#define DEFINE_SENSOR_LABEL_LINE(SENSOR_INDEX, LINE_INDEX, FLAGS) \
const FixedLabelData sensor##SENSOR_INDEX##line##LINE_INDEX PROGMEM = \
{ \
	{ms_historyX + GRAPH_NUMPOINTS + 2, ms_groupsStartY + (SENSOR_INDEX * (GRAPH_HEIGHT + ms_spaceBetweenGroups)) + (LINE_INDEX * (GRAPH_HEIGHT/3)), 30, GRAPH_HEIGHT/3}, \
	HAlign::Center, VAlign::Center, \
	TINY_FONT, \
	DARKGREY, BLACK, \
	GFX_FLAG_ERASEBKG | FLAGS \
};

#define DEFINE_SENSOR_LABELS(SENSOR_INDEX) \
	DEFINE_SENSOR_LABEL_LINE(SENSOR_INDEX, 0, 0) \
	DEFINE_SENSOR_LABEL_LINE(SENSOR_INDEX, 1, GFX_FLAG_MUMASPERCENTAGE) \
	DEFINE_SENSOR_LABEL_LINE(SENSOR_INDEX, 2, 0) \

DEFINE_SENSOR_LABELS(0)
DEFINE_SENSOR_LABELS(1)
DEFINE_SENSOR_LABELS(2)
DEFINE_SENSOR_LABELS(3)


FixedNumLabel sensorLabels[NUM_MOISTURESENSORS][3] =
{
	{
		{ &sensor0line0 },
		{ &sensor0line1 },
		{ &sensor0line2 }
	}
#if NUM_MOISTURESENSORS>1
	,
	{
		{ &sensor1line0 },
		{ &sensor1line1 },
		{ &sensor1line2 }
	}
#endif
#if NUM_MOISTURESENSORS>2
	,
	{
		{ &sensor2line0 },
		{ &sensor2line1 },
		{ &sensor2line2 }
	}
#endif
#if NUM_MOISTURESENSORS>3
	,
	{
		{ &sensor3line0 },
		{ &sensor3line1 },
		{ &sensor3line2 }
	}
#endif
};

}


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
		gScreen.fillScreen(BLACK);
	}
#endif

	return 1.0f / 30.0f;
}
	

void DisplayTFT::changeToState(State newState)
{
    CZ_LOG(logDefault, Log, F("DisplayTFT::%s %dms %s->%s")
        , __FUNCTION__
		, (int)(m_timeInState * 1000.f)
        , ms_stateNames[(int)m_state]
        , ms_stateNames[(int)newState]);

    onLeaveState();
	gScreen.fillScreen(BLACK);
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


void DisplayTFT::plotHistory(int16_t x, int16_t y, int16_t h, const TFixedCapacityQueue<GraphPoint>& data, int previousDrawOffset, uint8_t valThreshold /*, const GraphPoint* oldData, int oldCount*/)
{
	CZ_ASSERT(previousDrawOffset<=0);

	// If the previous draw offset is 0, it means the graph data hasn't changed, so no need to redraw
	if (previousDrawOffset==0)
	{
		return;
	}
	int bottomY = y + h - 1;

	const int count = data.size();
	int oldDrawIdx = previousDrawOffset;
	for(int i=0; i<count; i++)
	{

		GraphPoint p = data.getAtIndex(i);
		int xx = x + i;

		bool doDrawLevel = false;
		bool doDrawMotor = false;

		if (oldDrawIdx >= 0)
		{
			GraphPoint oldPoint = data.getAtIndex(oldDrawIdx);
			if (oldPoint.val != p.val)
			{
				int yy = oldPoint.val;
				// erase previous dot
				gScreen.drawPixel(xx, bottomY - yy, BLACK);
				doDrawLevel = true;
			}

			if (oldPoint.on != p.on)
			{
				doDrawMotor = true;
			}
		}
		else
		{
			gScreen.drawFastVLine(xx, y, h, BLACK);
			doDrawLevel = true;
			doDrawMotor = true;
		}
		oldDrawIdx++;

		// The height for the plotting is h-1 because we reserve the top pixel for the motor on/off
		//int yy = map(p.val, 0, 100, 0,  h - 2);
		if (doDrawLevel)
		{
			int yy = p.val;
			gScreen.drawPixel(xx, bottomY - yy, p.val < valThreshold ? GRAPH_MOISTURE_LOW_COLOUR : GRAPH_MOISTURE_OK_COLOUR);
		}

		if (doDrawMotor)
		{
			gScreen.drawPixel(xx, y, p.on ? GRAPH_MOTOR_ON_COLOUR : GRAPH_MOTOR_OFF_COLOUR);
		}
	}

}


void DisplayTFT::onEnterState()
{
	switch(m_state)
	{
	case State::Initializing:
		{
			initializeScreen();
		}
		break;

	case State::Intro:
		{
			StaticLabel(&introLabel1_P).draw();
			StaticLabel(&introLabel2_P).draw();
			StaticLabel(&introLabel3_P).draw();

			gScreen.drawRGBBitmap(50,50, chart_curve, chart_curve_width, chart_curve_width);

			drawRGBBitmap(50,70, chart_curve, chart_curve_bitmask, chart_curve_width, chart_curve_height, WHITE);
			drawRGBBitmap(50,90, chart_curve, chart_curve_bitmask, chart_curve_width, chart_curve_height, YELLOW);
		}
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
		int x = ms_historyX;
		int y = ms_groupsStartY + (i*(GRAPH_HEIGHT + ms_spaceBetweenGroups));
		gScreen.drawRect(x-1, y-1, GRAPH_NUMPOINTS+2, GRAPH_HEIGHT+2, VERYDARKGREY);

		constexpr int16_t h = GRAPH_HEIGHT;
		int bottomY = y + h - 1;
		gScreen.drawFastHLine(x-3, bottomY - map(0, 0, 100, 0, h - 1), 4, DARKGREY);
		gScreen.drawFastHLine(x-3, bottomY - map(20, 0, 100, 0, h - 1), 4, DARKGREY);
		gScreen.drawFastHLine(x-3, bottomY - map(40, 0, 100, 0, h - 1), 4, DARKGREY);
		gScreen.drawFastHLine(x-3, bottomY - map(60, 0, 100, 0, h - 1), 4, DARKGREY);
		gScreen.drawFastHLine(x-3, bottomY - map(80, 0, 100, 0, h - 1), 4, DARKGREY);
		gScreen.drawFastHLine(x-3, bottomY - map(100, 0, 100, 0, h - 1), 4, DARKGREY);
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

		int y = ms_groupsStartY + (i*(GRAPH_HEIGHT + ms_spaceBetweenGroups));
		//
		// Draw history
		//
		{
			PROFILE_SCOPE(F("plotHistory"));

			int x = ms_historyX;
			plotHistory(x, y, GRAPH_HEIGHT, history, -data.getChangedCount(), data.getPercentageThreshold());
		}

		data.resetChanged();

		//
		// Draw values
		//
		{
			PROFILE_SCOPE(F("drawTextValues"));
			sensorLabels[i][0].setValueAndDraw(data.getWaterValue());
			sensorLabels[i][1].setValueAndDraw(data.getPercentageValue());
			sensorLabels[i][2].setValueAndDraw(data.getAirValue());
		}

	}
	

}

} // namespace cz
