#include "DisplayTFT.h"
#include "Context.h"
#include "Utils.h"
#include "crazygaze/micromuc/Logging.h"
#include "crazygaze/micromuc/StringUtils.h"
#include <crazygaze/micromuc/Profiler.h>
#include "Icons.h"
#include "DisplayCommon.h"


#define YP A3  // must be an analog pin, use "An" notation!
#define XM A2  // must be an analog pin, use "An" notation!
#define YM 9   // can be a digital pin
#define XP 8   // can be a digital pin

#define TS_MINX 108
#define TS_MINY 84
#define TS_MAXX 910
#define TS_MAXY 888

#define TS_MIN_PRESSURE 100
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
	{
		{10, 10, SCREEN_WIDTH - 20, SCREEN_HEIGHT - 20 }, // box
		HAlign::Left, VAlign::Top,
		LARGE_FONT,
		INTRO_TEXT_COLOUR,
		SCREEN_BKG_COLOUR,
		WidgetFlag::None
	},
	(const __FlashStringHelper*)introLabel1_value_P
};

const char introLabel2_value_P[] PROGMEM = "By";
const StaticLabelData introLabel2_P PROGMEM =
{
	{
		{10, 10, SCREEN_WIDTH - 20, SCREEN_HEIGHT - 20 }, // box
		HAlign::Center, VAlign::Center,
		LARGE_FONT,
		INTRO_TEXT_COLOUR,
		SCREEN_BKG_COLOUR,
		WidgetFlag::None
	},
	(const __FlashStringHelper*)introLabel2_value_P
};

const char introLabel3_value_P[] PROGMEM = "Rui Figueira";
const StaticLabelData introLabel3_P PROGMEM =
{
	{
		{10, 10, SCREEN_WIDTH - 20, SCREEN_HEIGHT - 20 }, // box
		HAlign::Right, VAlign::Bottom,
		LARGE_FONT,
		INTRO_TEXT_COLOUR,
		SCREEN_BKG_COLOUR,
		WidgetFlag::None
	},
	(const __FlashStringHelper*)introLabel3_value_P
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
	GRAPH_VALUES_TEXT_COLOUR, GRAPH_VALUES_BKG_COLOUR, \
	WidgetFlag::EraseBkg | FLAGS \
};

#define DEFINE_SENSOR_LABELS(SENSOR_INDEX) \
	DEFINE_SENSOR_LABEL_LINE(SENSOR_INDEX, 0, WidgetFlag::None) \
	DEFINE_SENSOR_LABEL_LINE(SENSOR_INDEX, 1, WidgetFlag::NumAsPercentage) \
	DEFINE_SENSOR_LABEL_LINE(SENSOR_INDEX, 2, WidgetFlag::None) \

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

DisplayTFT::OverviewState::OverviewState(DisplayTFT& outer)
	: DisplayState(outer)
{
	for(int idx = 0; idx<NUM_MOISTURESENSORS; idx++)
	{
		m_groupGraphs[idx].init(idx);
	}
}

void DisplayTFT::OverviewState::init()
{
	m_sensorMainMenu.init();
}

void DisplayTFT::OverviewState::tick(float deltaSeconds)
{
	m_sensorMainMenu.tick(deltaSeconds);
	draw();
}

void DisplayTFT::OverviewState::onEnter()
{
	m_forceRedraw = true;
	memset(m_sensorUpdates, 0, sizeof(m_sensorUpdates));
	m_sensorMainMenu.setForceDraw();
}

void DisplayTFT::OverviewState::onLeave()
{
}

void DisplayTFT::OverviewState::onEvent(const Event& evt)
{
	m_sensorMainMenu.onEvent(evt);
	
	for(GroupGraph& w : m_groupGraphs)
	{
		w.onEvent(evt);
	}

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

		default:
		break;
	}
	
}

void DisplayTFT::OverviewState::draw()
{
	PROFILE_SCOPE(F("OverviewState::drawOverview"));

	for(int i=0; i<NUM_MOISTURESENSORS; i++)
	{
		PROFILE_SCOPE(F("groupDrawing"));

		m_groupGraphs[i].draw(m_forceRedraw);

		GroupData& data = gCtx.data.getGroupData(i);

		//
		// Draw values
		//
		if (m_sensorUpdates[i])
		{
			PROFILE_SCOPE(F("drawTextValues"));
			Overview::sensorLabels[i][0].setValueAndDraw(data.getWaterValue(), m_forceRedraw);
			Overview::sensorLabels[i][1].setValueAndDraw(data.getPercentageValue(), m_forceRedraw);
			Overview::sensorLabels[i][2].setValueAndDraw(data.getAirValue(), m_forceRedraw);
			m_sensorUpdates[i] = 0;
		}
	}

	m_forceRedraw = false;
}

//
// SensorMainMenu
//
void SensorMainMenu::init()
{
	auto initButton = [this](ButtonID id, auto&&... params)
	{
		m_buttons[(int)id].init((int)id, std::forward<decltype(params)>(params)...);
	};

	initButton(ButtonID::StartGroup, Overview::getMenuButtonPos(0,0), SCREEN_BKG_COLOUR, img_Play);
	initButton(ButtonID::StopGroup, Overview::getMenuButtonPos(0,0), SCREEN_BKG_COLOUR, img_Stop);
	initButton(ButtonID::Shot, Overview::getMenuButtonPos(1,0), SCREEN_BKG_COLOUR, img_Shot);
	initButton(ButtonID::Settings, Overview::getMenuButtonPos(2,0), SCREEN_BKG_COLOUR, img_Settings);

	enable();
}

void SensorMainMenu::tick(float deltaSeconds)
{
	draw();
}

void SensorMainMenu::updateButtons()
{
	bool isGroupSelected = gCtx.data.getSelectedGroup()==-1 ? false : true;
	bool groupIsRunning = isGroupSelected ? gCtx.data.getGroupData(gCtx.data.getSelectedGroup()).isRunning() : false;

	m_buttons[(int)ButtonID::StartGroup].setVisible(!(isGroupSelected && groupIsRunning));
	m_buttons[(int)ButtonID::StartGroup].setEnabled(isGroupSelected && !groupIsRunning);

	m_buttons[(int)ButtonID::StopGroup].setVisible(isGroupSelected && groupIsRunning);
	m_buttons[(int)ButtonID::Shot].setEnabled(isGroupSelected);
	m_buttons[(int)ButtonID::Settings].setEnabled(isGroupSelected);
}

void SensorMainMenu::onEvent(const Event& evt)
{
	switch (evt.type)
	{
		case Event::GroupSelected:
		case Event::GroupOnOff:
		case Event::ConfigLoad:
		{
			updateButtons();
		}
		break;
	}
}

void SensorMainMenu::draw()
{
	for(gfx::ImageButton& btn : m_buttons)
	{
		btn.draw(m_forceDraw);
	}
	m_forceDraw = false;
}

void SensorMainMenu::enable()
{
	for(gfx::ImageButton& btn : m_buttons)
	{
		btn.setEnabled(true);
		btn.setClearWhenHidden(false);
	}

	updateButtons();
}

void SensorMainMenu::disable()
{
	for(gfx::ImageButton& btn : m_buttons)
	{
		btn.setEnabled(false);
	}
}
	


//////////////////////////////////////////////////////////////////////////
// DisplayTFT
//////////////////////////////////////////////////////////////////////////


DisplayTFT::DisplayTFT()
	// For better pressure precision, we need to know the resistance
	// between X+ and X- Use any multimeter to read it
	// For the one we're using, its 300 ohms across the X plate
	: m_ts(XP, YP, XM, YM, 300)
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

	updateTouch();

	//CZ_LOG(logDefault, Log, F("DisplayTFT::%s: state=%s, timeInState = %d"), __FUNCTION__, ms_stateNames[(int)m_state], (int)m_timeInState);
	m_state->tick(deltaSeconds);

	return 1.0f / 30.0f;
}

void DisplayTFT::updateTouch()
{
	TSPoint p = m_ts.getPoint();
	pinMode(YP, OUTPUT); //restore shared pins
	pinMode(XM, OUTPUT);

	if (p.z > TS_MIN_PRESSURE)
	{
		//CZ_LOG(logDefault, Log, F("Touch=(%3d,%3d,%3d)"), p.x, p.y, p.z);
	}

	m_touch.pressed = false;
	// If we are not touching right now, but were in the previous call, that means we have a press event
	if (p.z < TS_MIN_PRESSURE && m_touch.tmp.z >= TS_MIN_PRESSURE)
	{
		// Map to screen coordinates
		m_touch.pos.x = map(m_touch.tmp.y, TS_MINY, TS_MAXY, 0, gScreen.width());
		m_touch.pos.y = map(m_touch.tmp.x, TS_MAXX, TS_MINX, 0, gScreen.height());
		m_touch.pressed = true;
		CZ_LOG(logDefault, Log, F("Press=(%3d,%3d)"), m_touch.pos.x, m_touch.pos.y);
	}

	m_touch.tmp = p;
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

	gScreen.fillScreen(SCREEN_BKG_COLOUR);
	m_state = &newState;
    m_timeInState = 0.0f;
	m_state->onEnter();
}


} // namespace cz
