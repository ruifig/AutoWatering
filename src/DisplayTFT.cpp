#include "DisplayTFT.h"
#include "Utils.h"
#include "crazygaze/micromuc/Logging.h"
#include "crazygaze/micromuc/StringUtils.h"
#include <crazygaze/micromuc/Profiler.h>
#include "Icons.h"


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


constexpr Rect getHistoryPlotRect(int index)
{
	Rect rect{0};
	// Don't start a 0, so we leave space to add the small markers
	rect.x = 3;
	// 1+ to leave a line for the box
	// + X at the end to add a space between different history plots
	rect.y = 1 + (index*(GRAPH_HEIGHT + 5));
	rect.height = GRAPH_HEIGHT;
	rect.width = GRAPH_NUMPOINTS;
	return rect;
}

// Make sure we have enough space at the bottom for the menu
static_assert((SCREEN_HEIGHT - (getHistoryPlotRect(3).y + getHistoryPlotRect(3).height)) > 64, "Need enough space left at the bototm for the menu (64 pixels high)");

constexpr Pos getMenuButtonPos(uint8_t col, uint8_t row)
{
	Pos pos{0};
	pos.x = (32 + 3) * col;
	pos.y = getHistoryPlotRect(3).y + getHistoryPlotRect(3).height + 3 + (32 + 3) * row;
	return pos;
}


//
// Intro screen
//
const char introLabel1_value_P[] PROGMEM = "AutoWatering";
const StaticLabelData introLabel1_P PROGMEM =
{
	{10, 10, SCREEN_WIDTH - 20, SCREEN_HEIGHT - 20 }, // box
	HAlign::Left, VAlign::Top,  (const __FlashStringHelper*)introLabel1_value_P,
	LARGE_FONT,
	Colour_Green,
	Colour_Black,
	GFX_FLAG_ERASEBKG
};

const char introLabel2_value_P[] PROGMEM = "By";
const StaticLabelData introLabel2_P PROGMEM =
{
	{10, 10, SCREEN_WIDTH - 20, SCREEN_HEIGHT - 20 }, // box
	HAlign::Center, VAlign::Center,  (const __FlashStringHelper*)introLabel2_value_P,
	LARGE_FONT,
	Colour_Green,
	Colour_Black,
	0
};

const char introLabel3_value_P[] PROGMEM = "Rui Figueira";
const StaticLabelData introLabel3_P PROGMEM =
{
	{10, 10, SCREEN_WIDTH - 20, SCREEN_HEIGHT - 20 }, // box
	HAlign::Right, VAlign::Bottom,  (const __FlashStringHelper*)introLabel3_value_P,
	LARGE_FONT,
	Colour_Green,
	Colour_Black,
	0
};


//
// Sensor data (left to the history plot)
//
#define DEFINE_SENSOR_LABEL_LINE(SENSOR_INDEX, LINE_INDEX, FLAGS) \
const FixedLabelData sensor##SENSOR_INDEX##line##LINE_INDEX PROGMEM = \
{ \
	{ \
		getHistoryPlotRect(SENSOR_INDEX).x + getHistoryPlotRect(SENSOR_INDEX).width + 2, \
		getHistoryPlotRect(SENSOR_INDEX).y + (LINE_INDEX * (GRAPH_HEIGHT/3)),  \
		32, \
		GRAPH_HEIGHT/3, \
	}, \
	HAlign::Center, VAlign::Center, \
	TINY_FONT, \
	Colour_DarkGrey, Colour_Black, \
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


//
// SensorMainMenu
//
void SensorMainMenu::init()
{
	m_buttons[Start].init(gScreen, img_Play, getMenuButtonPos(0,0), Colour_Black);
	m_buttons[Stop].init(gScreen, img_Stop, getMenuButtonPos(0,0), Colour_Black);
	m_buttons[Shot].init(gScreen, img_Shot, getMenuButtonPos(1,0), Colour_Black);
	m_buttons[Settings].init(gScreen, img_Settings, getMenuButtonPos(2,0), Colour_Black);
	m_buttons[Stop].setState(ButtonState::Hidden);
}

void SensorMainMenu::tick(float deltaSeconds)
{
}

void SensorMainMenu::draw(bool forceDraw)
{
	for(auto&& btn : m_buttons)
	{
		btn.draw(forceDraw);
	}
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

	m_sensorMainMenu.init();

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
		gScreen.fillScreen(Colour_Black);
	}
#endif

	m_forceDrawOnNextTick = false;
	return 1.0f / 30.0f;
}
	
void DisplayTFT::onEvent(const Event& evt)
{
	switch(evt.type)
	{
		case Event::ConfigLoad:
			changeToState(State::Overview);
			m_forceDrawOnNextTick = true;
		break;

		case Event::SoilMoistureSensorReading:
		{
			auto idx = static_cast<const SoilMoistureSensorReadingEvent&>(evt).index;

			// We only increment if < GRAPH_NUMPOINTS, so we don't wrap around to 0 if it happens the display is not updated
			// for a very long time (shouldn't happen), or if the sensors update too quickly before the screen updates (shouldn't happen).
			// This is needed because the number of sensor updates is used to optimize the sensor history drawing.
			if (m_soilMoistureSensorUpdates[idx]<GRAPH_NUMPOINTS)
			{
				++m_soilMoistureSensorUpdates[idx];
			}
		}
		break;

		case Event::StartGroup:
		{
			auto idx = static_cast<const StartGroupEvent&>(evt).index;

		}
		break;
	}
}

void DisplayTFT::changeToState(State newState)
{
    CZ_LOG(logDefault, Log, F("DisplayTFT::%s %ssec %s->%s")
        , __FUNCTION__
		, *FloatToString(m_timeInState)
        , ms_stateNames[(int)m_state]
        , ms_stateNames[(int)newState]);

    onLeaveState();
	gScreen.fillScreen(Colour_Black);
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
				gScreen.drawPixel(xx, bottomY - yy, Colour_Black);
				doDrawLevel = true;
			}

			if (oldPoint.on != p.on)
			{
				doDrawMotor = true;
			}
		}
		else
		{
			gScreen.drawFastVLine(xx, y, h, Colour_Black);
			doDrawLevel = true;
			doDrawMotor = true;
		}
		oldDrawIdx++;

		// The height for the plotting is h-1 because we reserve the top pixel for the motor on/off
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
		}
		break;

	case State::Overview:
		memset(m_soilMoistureSensorUpdates, 255, sizeof(m_soilMoistureSensorUpdates));
		drawHistoryBoxes();
		m_sensorMainMenu.draw(true);
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
		Rect rect = getHistoryPlotRect(i);
		gScreen.drawRect(rect.x-1 , rect.y-1, rect.width+2, rect.height+2, Colour_VeryDarkGrey);

		constexpr int16_t h = GRAPH_HEIGHT;
		int bottomY = rect.y + rect.height - 1;
		gScreen.drawFastHLine(0, bottomY - map(0, 0, 100, 0, h - 1),   2, Colour_DarkGrey);
		gScreen.drawFastHLine(0, bottomY - map(20, 0, 100, 0, h - 1),  2, Colour_DarkGrey);
		gScreen.drawFastHLine(0, bottomY - map(40, 0, 100, 0, h - 1),  2, Colour_DarkGrey);
		gScreen.drawFastHLine(0, bottomY - map(60, 0, 100, 0, h - 1),  2, Colour_DarkGrey);
		gScreen.drawFastHLine(0, bottomY - map(80, 0, 100, 0, h - 1),  2, Colour_DarkGrey);
		gScreen.drawFastHLine(0, bottomY - map(100, 0, 100, 0, h - 1), 2, Colour_DarkGrey);
	}

	// #RVF : Remove this
	#if 0
		#define TEST_ICON(idx, name) \
			drawRGBBitmap_P(32*idx,SCREEN_HEIGHT-32*2 - 3, img_##name, img_##name##_bitmask, img_##name##_width, img_##name##_height, PINK); \
			drawRGBBitmapDisabled_P(32*idx,SCREEN_HEIGHT-32*1, img_##name, img_##name##_bitmask, img_##name##_width, img_##name##_height, PINK);

		TEST_ICON(0, Play);
		TEST_ICON(1, Stop);
		TEST_ICON(2, Shot);
		TEST_ICON(3, Calibrate);
		TEST_ICON(4, Settings);
		TEST_ICON(5, Save);
		TEST_ICON(6, SetSensorInterval);
		TEST_ICON(7, SetWateringDuration);
		TEST_ICON(8, Close);
		TEST_ICON(9, SetThreshold);

		TEST_ICON(0, Add);
		TEST_ICON(1, Remove);
	#endif


}

void DisplayTFT::drawOverview()
{
	PROFILE_SCOPE(F("DisplayTFT:drawOverview"));

	for(int i=0; i<NUM_MOISTURESENSORS; i++)
	{
		if (m_soilMoistureSensorUpdates[i]==0)
		{
			continue;
		}

		PROFILE_SCOPE(F("sensorDrawing"));

		GroupData& data = m_ctx.data.getGroupData(i);
		const HistoryQueue& history = data.getHistory();

		//
		// Draw history
		//
		{
			PROFILE_SCOPE(F("plotHistory"));
			Rect rect = getHistoryPlotRect(i);
			// #RVF : getPercentageThreshold (the last parameter) won't work properly, because we are passing 0..100, but the GraphPoint doesn't use a value of 0..100
			plotHistory(rect.x, rect.y, GRAPH_HEIGHT, history, -static_cast<int>(m_soilMoistureSensorUpdates[i]), data.getPercentageThreshold());
		}

		//
		// Draw values
		//
		{
			PROFILE_SCOPE(F("drawTextValues"));
			sensorLabels[i][0].setValueAndDraw(data.getWaterValue(), m_forceDrawOnNextTick);
			sensorLabels[i][1].setValueAndDraw(data.getPercentageValue(), m_forceDrawOnNextTick);
			sensorLabels[i][2].setValueAndDraw(data.getAirValue(), m_forceDrawOnNextTick);
		}

	}

	memset(m_soilMoistureSensorUpdates, 0, sizeof(m_soilMoistureSensorUpdates));

}

} // namespace cz
