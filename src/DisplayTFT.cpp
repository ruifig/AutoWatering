#include "DisplayTFT.h"
#include "Context.h"
#include "crazygaze/micromuc/Logging.h"
#include "crazygaze/micromuc/StringUtils.h"
#include <crazygaze/micromuc/Profiler.h>
#include "Icons.h"
#include "gfx/MyDisplay1.h"
#include "DisplayCommon.h"

void doGroupShot(uint8_t index);

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

extern MyDisplay1 gScreen;

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
static_assert((SCREEN_HEIGHT - (getHistoryPlotRect(3).y + getHistoryPlotRect(3).height)) > 64, "Need enough space left at the bottom for the menu (64 pixels high)");

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
const LabelData sensor##SENSOR_INDEX##line##LINE_INDEX PROGMEM = \
{ \
	{ \
		getHistoryPlotRect(SENSOR_INDEX).x + getHistoryPlotRect(SENSOR_INDEX).width + 2, \
		getHistoryPlotRect(SENSOR_INDEX).y + (LINE_INDEX * (GRAPH_HEIGHT/3)),  \
		SCREEN_WIDTH - (getHistoryPlotRect(SENSOR_INDEX).x + getHistoryPlotRect(SENSOR_INDEX).width + 2), \
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

NumLabel<true> sensorLabels[NUM_PAIRS][3] =
{
	{
		{ sensor0line0 },
		{ sensor0line1 },
		{ sensor0line2 }
	}
#if NUM_PAIRS>1
	,
	{
		{ sensor1line0 },
		{ sensor1line1 },
		{ sensor1line2 }
	}
#endif
#if NUM_PAIRS>2
	,
	{
		{ sensor2line0 },
		{ sensor2line1 },
		{ sensor2line2 }
	}
#endif
#if NUM_PAIRS>3
	,
	{
		{ sensor3line0 },
		{ sensor3line1 },
		{ sensor3line2 }
	}
#endif
};

} } // namespace Overview

DisplayTFT::OverviewState::OverviewState(DisplayTFT& outer)
	: DisplayState(outer)
{
}

void DisplayTFT::OverviewState::init()
{
	for(int idx = 0; idx<NUM_PAIRS; idx++)
	{
		m_groupGraphs[idx].init(idx);
	}

	m_sensorMainMenu.init();
	m_settingsMenu.init();

}

void DisplayTFT::OverviewState::tick(float deltaSeconds)
{

	// process touch
	if (m_outer.m_touch.pressed)
	{
		bool consumed = false;

		for(int8_t idx = 0; idx<NUM_PAIRS; idx++)
		{
			if (m_groupGraphs[idx].contains(m_outer.m_touch.pos))
			{
				gCtx.data.trySetSelectedGroup(idx);
				consumed = true;
				break;
			}
		}

		if (!consumed)
		{
			if (m_inSettingsMenu)
			{
				consumed = m_settingsMenu.processTouch(m_outer.m_touch.pos);
			}
			else
			{
				consumed = m_sensorMainMenu.processTouch(m_outer.m_touch.pos);
			}
		}

	}

	if (!m_inSettingsMenu)
	{
		// Doing this before the tick, so if we switch to the Settings menu, the main menu has a chance to be drawn as disabled
		if (m_sensorMainMenu.checkShowSettings())
		{
			CZ_LOG(logDefault, Log, F("Switching to settings"));
			m_sensorMainMenu.hide();	
			m_settingsMenu.show();
			m_inSettingsMenu = true;
		}

		m_sensorMainMenu.tick(deltaSeconds);
	}

	if (m_inSettingsMenu)
	{
		bool doSave;
		if (m_settingsMenu.checkClose(doSave))
		{
			CZ_LOG(logDefault, Log, F("Switching to main menu"));
			m_settingsMenu.hide();
			m_sensorMainMenu.show();
			m_inSettingsMenu = false;
		}

		m_settingsMenu.tick(deltaSeconds);
	}
	
	draw();
}

void DisplayTFT::OverviewState::onEnter()
{
	m_forceRedraw = true;
	m_inSettingsMenu = false;
	memset(m_sensorUpdates, 0, sizeof(m_sensorUpdates));
	m_sensorMainMenu.setForceDraw();
	m_settingsMenu.setForceDraw();
}

void DisplayTFT::OverviewState::onLeave()
{
}

void DisplayTFT::OverviewState::onEvent(const Event& evt)
{
	if (m_inSettingsMenu)
	{
		m_settingsMenu.onEvent(evt);
	}
	else
	{
		m_sensorMainMenu.onEvent(evt);
	}
	
	for(GroupGraph& w : m_groupGraphs)
	{
		w.onEvent(evt);
	}

	switch(evt.type)
	{
		case Event::ConfigLoad:
		{
			int8_t group = static_cast<const ConfigLoadEvent&>(evt).group;
			if (group == -1)
			{
				m_forceRedraw = true;
			}
			else
			{
				// Force an update to the group labels
				m_sensorUpdates[group]++;
			}
		}
		break;

		case Event::GroupOnOff:
		{
			const GroupOnOffEvent& e = static_cast<const GroupOnOffEvent&>(evt);
			// Force an update to the group labels;
			m_sensorUpdates[e.index]++;
		}
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

	for(int i=0; i<NUM_PAIRS; i++)
	{
		PROFILE_SCOPE(F("groupDrawing"));

		m_groupGraphs[i].draw(m_forceRedraw);

		GroupData& data = gCtx.data.getGroupData(i);

		//
		// Draw values
		//
		if (m_sensorUpdates[i] || m_forceRedraw)
		{
			if (data.isRunning())
			{
				PROFILE_SCOPE(F("drawTextValues"));
				Overview::sensorLabels[i][0].setValueAndDraw(data.getWaterValue(), m_forceRedraw);
				Overview::sensorLabels[i][1].setValueAndDraw(data.getPercentageValue(), m_forceRedraw);
				Overview::sensorLabels[i][2].setValueAndDraw(data.getAirValue(), m_forceRedraw);
			}
			else
			{
				for(auto&& label : Overview::sensorLabels[i])
				{
					label.clearValueAndDraw(m_forceRedraw);
				}
			}
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
	m_showSettings = false;
	
	auto initButton = [this](ButtonId id, auto&&... params)
	{
		m_buttons[(int)id].init((int)id, std::forward<decltype(params)>(params)...);
		m_buttons[(int)id].setClearWhenHidden(false);
	};

	initButton(ButtonId::StartGroup, Overview::getMenuButtonPos(0,0), SCREEN_BKG_COLOUR, img_Play);
	initButton(ButtonId::StopGroup, Overview::getMenuButtonPos(0,0), SCREEN_BKG_COLOUR, img_Stop);
	initButton(ButtonId::Shot, Overview::getMenuButtonPos(1,0), SCREEN_BKG_COLOUR, img_Shot);
	initButton(ButtonId::Settings, Overview::getMenuButtonPos(2,0), SCREEN_BKG_COLOUR, img_Settings);

	show();
}

void SensorMainMenu::tick(float deltaSeconds)
{
	draw();
}

bool SensorMainMenu::processTouch(const Pos& pos)
{
	{
		ImageButton& btn = m_buttons[(int)ButtonId::StartGroup];
		if (btn.canAcceptInput() && btn.contains(pos))
		{
			CZ_ASSERT(gCtx.data.hasGroupSelected());
			gCtx.data.getSelectedGroup()->setRunning(true);
			return true;
		}
	}
	
	{
		ImageButton& btn = m_buttons[(int)ButtonId::StopGroup];
		if (btn.canAcceptInput() && btn.contains(pos))
		{
			CZ_ASSERT(gCtx.data.hasGroupSelected());
			gCtx.data.getSelectedGroup()->setRunning(false);
			return true;
		}
	}

	{
		ImageButton& btn = m_buttons[(int)ButtonId::Shot];
		if (btn.canAcceptInput() && btn.contains(pos))
		{
			CZ_ASSERT(gCtx.data.hasGroupSelected());
			GroupData* data = gCtx.data.getSelectedGroup();
			doGroupShot(data->getIndex());
			return true;
		}
	}
	
	{
		ImageButton& btn = m_buttons[(int)ButtonId::Settings];
		if (btn.canAcceptInput() && btn.contains(pos))
		{
			CZ_ASSERT(gCtx.data.hasGroupSelected());
			m_showSettings = true;
			return true;
		}
	}

	return false;
}

void SensorMainMenu::updateButtons()
{
	bool isGroupSelected = gCtx.data.hasGroupSelected();
	bool groupIsRunning = isGroupSelected ? gCtx.data.getSelectedGroup()->isRunning() : false;

	m_buttons[(int)ButtonId::StartGroup].setVisible(!(isGroupSelected && groupIsRunning));
	m_buttons[(int)ButtonId::StartGroup].setEnabled(isGroupSelected && !groupIsRunning);

	m_buttons[(int)ButtonId::StopGroup].setVisible(isGroupSelected && groupIsRunning);

	// Shot and Settings stay enabled even if the group is not running. This is intentional
	m_buttons[(int)ButtonId::Shot].setEnabled(isGroupSelected);
	m_buttons[(int)ButtonId::Settings].setEnabled(isGroupSelected);
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

void SensorMainMenu::show()
{
	for(gfx::ImageButton& btn : m_buttons)
	{
		btn.setVisible(true);
	}

	updateButtons();
}

void SensorMainMenu::hide()
{
	for(gfx::ImageButton& btn : m_buttons)
	{
		btn.setVisible(false);
	}
}

bool SensorMainMenu::checkShowSettings()
{
	if (m_showSettings)
	{
		m_showSettings = false;
		return true;
	}
	else
	{
		return false;
	}
}

//
// SettingsMenu
//

#define DEFINE_SENSOR_CALIBRATION_LABEL_LINE(LINE_INDEX, FLAGS) \
const LabelData sensorCalibrationSettings##LINE_INDEX PROGMEM = \
{ \
	{Overview::getMenuButtonPos(1, 1).x, Overview::getMenuButtonPos(1,1).y + (LINE_INDEX * (GRAPH_HEIGHT/3)), 32, GRAPH_HEIGHT/3}, \
	HAlign::Center, VAlign::Center, \
	TINY_FONT, \
	GRAPH_VALUES_TEXT_COLOUR, GRAPH_VALUES_BKG_COLOUR, \
	WidgetFlag::EraseBkg | FLAGS \
};

DEFINE_SENSOR_CALIBRATION_LABEL_LINE(0, WidgetFlag::None)
DEFINE_SENSOR_CALIBRATION_LABEL_LINE(1, WidgetFlag::NumAsPercentage)
DEFINE_SENSOR_CALIBRATION_LABEL_LINE(2, WidgetFlag::None)

#if 0
#define DEFINE_SENSOR_LABEL_LINE(SENSOR_INDEX, LINE_INDEX, FLAGS) \
const LabelData sensor##SENSOR_INDEX##line##LINE_INDEX PROGMEM = \
{ \
	{ \
		getHistoryPlotRect(SENSOR_INDEX).x + getHistoryPlotRect(SENSOR_INDEX).width + 2, \
		getHistoryPlotRect(SENSOR_INDEX).y + (LINE_INDEX * (GRAPH_HEIGHT/3)),  \
		SCREEN_WIDTH - (getHistoryPlotRect(SENSOR_INDEX).x + getHistoryPlotRect(SENSOR_INDEX).width + 2), \
		GRAPH_HEIGHT/3, \
	}, \
	HAlign::Center, VAlign::Center, \
	TINY_FONT, \
	GRAPH_VALUES_TEXT_COLOUR, GRAPH_VALUES_BKG_COLOUR, \
	WidgetFlag::EraseBkg | FLAGS \
};
#endif

SettingsMenu::SettingsMenu() :
	m_sensorLabels
	{
		{ sensorCalibrationSettings0},
		{ sensorCalibrationSettings1},
		{ sensorCalibrationSettings2},
	}
{
}

void SettingsMenu::init()
{
	auto initButton = [this](ButtonId id, auto&&... params)
	{
		m_buttons[(int)id].init((int)id, std::forward<decltype(params)>(params)...);
		m_buttons[(int)id].setClearWhenHidden(true);
	};

	// First line
	initButton(ButtonId::CloseAndSave, Overview::getMenuButtonPos(0,0), SCREEN_BKG_COLOUR, img_Save);
	initButton(ButtonId::Calibrate, Overview::getMenuButtonPos(1,0), SCREEN_BKG_COLOUR, img_Ruler);
	initButton(ButtonId::SensorInterval, Overview::getMenuButtonPos(2,0), SCREEN_BKG_COLOUR, img_SetSensorInterval);
	initButton(ButtonId::ShotDuration, Overview::getMenuButtonPos(3,0), SCREEN_BKG_COLOUR, img_SetWateringDuration);
	// Leaving one empty grid space intentionally, so the CloseAndIgnore button is spaced away from the others
	initButton(ButtonId::CloseAndIgnore, Overview::getMenuButtonPos(5,0), SCREEN_BKG_COLOUR, img_Close);

	// Second line
	initButton(ButtonId::SetGroupThreshold, Overview::getMenuButtonPos(2,1), SCREEN_BKG_COLOUR, img_SetThreshold);
	initButton(ButtonId::Minus, Overview::getMenuButtonPos(1,1), SCREEN_BKG_COLOUR, img_Remove);
	initButton(ButtonId::Plus, Overview::getMenuButtonPos(3,1), SCREEN_BKG_COLOUR, img_Add);

	//hide();
}

void SettingsMenu::tick(float deltaSeconds)
{
	draw();
}

bool SettingsMenu::processTouch(const Pos& pos)
{
	CZ_ASSERT(gCtx.data.hasGroupSelected());

	{
		ImageButton& btn = m_buttons[(int)ButtonId::CloseAndSave];
		if (btn.canAcceptInput() && btn.contains(pos))
		{
			m_pressedId = ButtonId::CloseAndSave;
			return true;
		}
	}
	
	{
		ImageButton& btn = m_buttons[(int)ButtonId::CloseAndIgnore];
		if (btn.canAcceptInput() && btn.contains(pos))
		{
			m_pressedId = ButtonId::CloseAndIgnore;
			return true;
		}
	}

	return false;
}

#if 0
void SettingsMenu::updateButtons()
{
	bool isGroupSelected = gCtx.data.hasGroupSelected();
	bool groupIsRunning = isGroupSelected ? gCtx.data.getSelectedGroup()->isRunning() : false;

	m_buttons[(int)ButtonId::StartGroup].setVisible(!(isGroupSelected && groupIsRunning));
	m_buttons[(int)ButtonId::StartGroup].setEnabled(isGroupSelected && !groupIsRunning);

	m_buttons[(int)ButtonId::StopGroup].setVisible(isGroupSelected && groupIsRunning);

	// Shot and Settings stay enabled even if the group is not running. This is intentional
	m_buttons[(int)ButtonId::Shot].setEnabled(isGroupSelected);
	m_buttons[(int)ButtonId::Settings].setEnabled(isGroupSelected);
}
#endif

void SettingsMenu::onEvent(const Event& evt)
{
}

void SettingsMenu::draw()
{
	GroupData* data = gCtx.data.getSelectedGroup();

	for(gfx::ImageButton& btn : m_buttons)
	{
		btn.draw(m_forceDraw);
	}

	if (data)
	{
		if (m_state==State::Main || m_state==State::CalibratingSensor)
		{
			m_sensorLabels[0].setValueAndDraw(data->getWaterValue(), m_forceDraw);
			m_sensorLabels[1].setValueAndDraw(data->getPercentageValue(), m_forceDraw);
			m_sensorLabels[2].setValueAndDraw(data->getAirValue(), m_forceDraw);
		}
	}

	m_forceDraw = false;
}

void SettingsMenu::setButton(ButtonId idx, bool enabled, bool visible)
{
	m_buttons[(int)idx].setEnabled(enabled);
	m_buttons[(int)idx].setVisible(visible);
};

void SettingsMenu::setButtonRange(ButtonId first, ButtonId last, bool enabled, bool visible)
{
	for(int idx = (int)first; idx <= (int)last; idx++)
	{
		m_buttons[idx].setEnabled(enabled);
		m_buttons[idx].setVisible(visible);
	}
};

void SettingsMenu::show()
{
	// CloseAndSave is hidden until we actually enter a proper settings changing menu
	setButton(ButtonId::CloseAndSave, false, false);
	setButtonRange(ButtonId::Calibrate, ButtonId::CloseAndIgnore, true, true);

	setButtonRange(ButtonId::SetGroupThreshold, ButtonId::Plus, false, false);

	for(gfx::NumLabel<true>& label : m_sensorLabels)
	{
		label.clearValue();
	}

	gCtx.data.setInGroupConfigMenu(true);
}

void SettingsMenu::hide()
{
	setButtonRange(ButtonId::CloseAndSave, ButtonId::Plus, false, false);
	gCtx.data.setInGroupConfigMenu(false);
}

bool SettingsMenu::checkClose(bool& doSave)
{
	if (m_pressedId == ButtonId::CloseAndSave)
	{
		doSave = true;
		m_pressedId = ButtonId::Max;
		return true;
	}
	else if (m_pressedId == ButtonId::CloseAndIgnore)
	{
		doSave = false;
		m_pressedId = ButtonId::Max;
		return true;
	}

	return false;
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
		CZ_LOG(logDefault, Verbose, F("Press=(%3d,%3d)"), m_touch.pos.x, m_touch.pos.y);
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
