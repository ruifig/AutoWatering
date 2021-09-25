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

//////////////////////////////////////////////////////////////////////////
// InitializeState
//////////////////////////////////////////////////////////////////////////

void DisplayTFT::InitializeState::tick(float deltaSeconds)
{
	m_outer.changeToState(m_outer.m_states.intro);
}

void DisplayTFT::InitializeState::onEnter()
{
	initializeScreen();
}

void DisplayTFT::InitializeState::onLeave()
{
}

//////////////////////////////////////////////////////////////////////////
// IntroState
//////////////////////////////////////////////////////////////////////////

namespace { namespace Intro {

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

} } // namespace Intro


void DisplayTFT::IntroState::tick(float deltaSeconds)
{
	if (m_outer.m_timeInState >= INTRO_DURATION)
	{
		m_outer.changeToState(m_outer.m_states.overview);
	}
}

void DisplayTFT::IntroState::onEnter()
{
	StaticLabel(&Intro::introLabel1_P).draw();
	StaticLabel(&Intro::introLabel2_P).draw();
	StaticLabel(&Intro::introLabel3_P).draw();
}

void DisplayTFT::IntroState::onLeave()
{
}

//////////////////////////////////////////////////////////////////////////
// OverviewState
//////////////////////////////////////////////////////////////////////////

namespace { namespace Overview {

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

} } // namespace Overview

void DisplayTFT::OverviewState::init()
{
	m_sensorMainMenu.init();
}

void DisplayTFT::OverviewState::tick(float deltaSeconds)
{
	drawOverview();
	m_forceRedraw = false;
}

void DisplayTFT::OverviewState::onEnter()
{
	m_forceRedraw = true;
	memset(m_sensorUpdates, 0, sizeof(m_sensorUpdates));
	drawHistoryBoxes();
	m_sensorMainMenu.draw(true);
}

void DisplayTFT::OverviewState::onLeave()
{
}

void DisplayTFT::OverviewState::onEvent(const Event& evt)
{
	switch(evt.type)
	{
		case Event::ConfigLoad:
			m_forceRedraw = true;
		break;

		case Event::SoilMoistureSensorReading:
		{
			auto idx = static_cast<const SoilMoistureSensorReadingEvent&>(evt).index;
			m_sensorUpdates[idx]++;
		}
		break;

	}
}

void DisplayTFT::OverviewState::drawOverview()
{
	PROFILE_SCOPE(F("OverviewState::drawOverview"));

	for(int i=0; i<NUM_MOISTURESENSORS; i++)
	{
		if (!m_forceRedraw && m_sensorUpdates[i] == 0)
		{
			continue;
		}

		PROFILE_SCOPE(F("sensorDrawing"));

		GroupData& data = m_outer.m_ctx.data.getGroupData(i);
		const HistoryQueue& history = data.getHistory();

		//
		// Draw history
		//
		{
			PROFILE_SCOPE(F("plotHistory"));
			plotHistory(i);
		}

		//
		// Draw values
		//
		{
			PROFILE_SCOPE(F("drawTextValues"));
			Overview::sensorLabels[i][0].setValueAndDraw(data.getWaterValue(), m_forceRedraw);
			Overview::sensorLabels[i][1].setValueAndDraw(data.getPercentageValue(), m_forceRedraw);
			Overview::sensorLabels[i][2].setValueAndDraw(data.getAirValue(), m_forceRedraw);
		}

	}

}

void DisplayTFT::OverviewState::drawHistoryBoxes()
{
	PROFILE_SCOPE(F("OverviewState::drawHistoryBoxes"));

	for(int i=0; i<NUM_MOISTURESENSORS; i++)
	{
		Rect rect = Overview::getHistoryPlotRect(i);
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

void DisplayTFT::OverviewState::plotHistory(int groupIndex)
{
	constexpr int h = GRAPH_HEIGHT;
	Rect rect = Overview::getHistoryPlotRect(groupIndex);
	GroupData& data = m_outer.m_ctx.data.getGroupData(groupIndex);
	const HistoryQueue& history = data.getHistory();

	int bottomY = rect.y + h - 1;

	const int count = history.size();
	
	uint8_t done[GRAPH_NUMPOINTS+1];
	memset(done,255, sizeof(done));

	// If there were more than 1 update, we don't have the info to scroll. We need to redraw
	bool redraw = m_sensorUpdates[groupIndex] > 1 ? true : false;
	if (redraw)
	{
		CZ_LOG(logDefault, Warning, F("Too many sensor udpates (%u)"), m_sensorUpdates[groupIndex]);
	}
	redraw |= m_forceRedraw;

	done[0] = history.getAtIndex(0).val;

	for(int i=1; i<count; i++)
	{
		int xx = rect.x + i - 1;
		GraphPoint p = history.getAtIndex(i);
		GraphPoint oldp = history.getAtIndex(i-1);
		done[i] = p.val;

		bool drawMotor = false;
		bool drawLevel = false;

		if (redraw)
		{
			// erase vertical line
			gScreen.drawFastVLine(xx, rect.y, h, Colour_Black);
			drawMotor = true;
			drawLevel = true;
		}
		else
		{
			// Since the motor on/off info is an horizontal line, we don't need to erase the previous. We just paint the pixel
			// at the right color if it changed
			if (oldp.on != p.on)
			{
				drawMotor = true;
			}

			if (oldp.val != p.val)
			{
				// Erase previous moisture level point
				gScreen.drawPixel(xx, bottomY - oldp.val, Colour_Black);
				drawLevel = true;
			}
		}

		if (drawMotor)
		{
			// draw new motor on/off
			gScreen.drawPixel(xx, rect.y, p.on ? GRAPH_MOTOR_ON_COLOUR : GRAPH_MOTOR_OFF_COLOUR);
		}

		if (drawLevel)
		{
			// Draw new moisture level
			gScreen.drawPixel(xx, bottomY - p.val, GRAPH_MOISTURELEVEL_COLOUR);
		}
	}

	m_sensorUpdates[groupIndex] = 0;
	CZ_LOG(logDefault, Log, F("Done"));
}

//
// SensorMainMenu
//
void SensorMainMenu::init()
{
	m_buttons[Start].init(gScreen, img_Play, Overview::getMenuButtonPos(0,0), Colour_Black);
	m_buttons[Stop].init(gScreen, img_Stop, Overview::getMenuButtonPos(0,0), Colour_Black);
	m_buttons[Shot].init(gScreen, img_Shot, Overview::getMenuButtonPos(1,0), Colour_Black);
	m_buttons[Settings].init(gScreen, img_Settings, Overview::getMenuButtonPos(2,0), Colour_Black);
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
	


//////////////////////////////////////////////////////////////////////////
// DisplayTFT
//////////////////////////////////////////////////////////////////////////


DisplayTFT::DisplayTFT(Context& ctx)
	: m_ctx(ctx)
	// For better pressure precision, we need to know the resistance
	// between X+ and X- Use any multimeter to read it
	// For the one we're using, its 300 ohms across the X plate
	, m_ts(XP, YP, XM, YM, 300)
	, m_states(*this)
{
}

void DisplayTFT::begin()
{
	m_states.initialize.init();
	m_states.intro.init();
	m_states.overview.init();
	changeToState(m_states.initialize);
}

float DisplayTFT::tick(float deltaSeconds)
{
	PROFILE_SCOPE(F("DisplayTFT::tick"));

	m_timeInState += deltaSeconds;

	//CZ_LOG(logDefault, Log, F("DisplayTFT::%s: state=%s, timeInState = %d"), __FUNCTION__, ms_stateNames[(int)m_state], (int)m_timeInState);
	m_state->tick(deltaSeconds);

	return 1.0f / 30.0f;
}
	
void DisplayTFT::onEvent(const Event& evt)
{
	m_state->onEvent(evt);	
}

void DisplayTFT::changeToState(DisplayState& newState)
{
	// NOTE: We need to take into account m_state will not be yet set if we are starting up
#if CZ_LOG_ENABLED
	CZ_LOG(logDefault, Log, F("DisplayTFT::%s %ssec %s->%s")
		, __FUNCTION__
		, *FloatToString(m_timeInState)
		, m_state ? m_state->getName() : "NONE"
		, newState.getName());
#endif

	if (m_state)
	{
		m_state->onLeave();
	}

	gScreen.fillScreen(Colour_Black);
	m_state = &newState;
    m_timeInState = 0.0f;
	m_state->onEnter();
}


} // namespace cz
