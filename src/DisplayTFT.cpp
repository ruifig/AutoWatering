#include "DisplayTFT.h"
#include "Context.h"
#include "crazygaze/micromuc/Logging.h"
#include "crazygaze/micromuc/StringUtils.h"
#include <crazygaze/micromuc/Profiler.h>
#include "Icons.h"
#include "DisplayCommon.h"
#include "Timer.h"

void doGroupShot(uint8_t index);

#define YP A3  // must be an analog pin, use "An" notation!
#define XM A2  // must be an analog pin, use "An" notation!
#define YM 9   // can be a digital pin
#define XP 8   // can be a digital pin

#define TS_MIN_PRESSURE 50

namespace cz
{

extern MyDisplay1 gScreen;
extern MyXPT2046 gTs;
extern Timer gTimer;

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
	gScreen.fillScreen(Colour_Black);
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
		m_outer.changeToState(m_outer.m_states.bootMenu);
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
// BootMenuState
//////////////////////////////////////////////////////////////////////////

namespace { namespace BootMenu {
	constexpr Rect getResetButtonPos()
	{
		constexpr int width = 80;
		constexpr int height = 60;
		return { {(SCREEN_WIDTH-width)/2, (SCREEN_HEIGHT-height)/2}, width, height};
	}
	constexpr Rect getConfirmButtonPos()
	{
		constexpr int width = 200;
		constexpr int height = 40;
		return { {(SCREEN_WIDTH-width)/2, 10}, width, height};
	}

} }

void DisplayTFT::BootMenuState::init()
{
}

void DisplayTFT::BootMenuState::tick(float deltaSeconds)
{
	m_defaultConfigCountdown -= deltaSeconds;

	if (m_waitingResetConfirmation == false)
	{
		gScreen.setTextColor(Colour_Yellow, Colour_Black);
		printAligned(
			{{0, BootMenu::getResetButtonPos().bottom() + 10}, SCREEN_WIDTH, 40},
			HAlign::Center, VAlign::Center,
			F(" Loading saved config in... ")
		);
		printAligned(
			{{0, BootMenu::getResetButtonPos().bottom() + 40}, SCREEN_WIDTH, 40},
			HAlign::Center, VAlign::Center,
			formatString(F("%d"), static_cast<int>(m_defaultConfigCountdown)),
			true
		);

		if (m_defaultConfigCountdown<=0)
		{
			gCtx.data.load();
			m_outer.changeToState(m_outer.m_states.overview);
		}
		else if (m_outer.m_touch.pressed && BootMenu::getResetButtonPos().contains(m_outer.m_touch.pos))
		{
			m_waitingResetConfirmation = true;
			gScreen.fillScreen(Colour_Black);

			// We don't need anything fancy for the BootMenu, so we can draw the button once and be done with it.
			// NOTE: We are intentionally drawing the confirm button in a different position to avoid clicking confirm by accident.
			// As-in, the user really needs to be sure.
			TextButton btn;
			btn.init(
				0, MEDIUM_FONT, BootMenu::getConfirmButtonPos(),
				Colour_Red, Colour_LightGrey, Colour_Black,
				"Confirm Reset");
			btn.draw();

			gScreen.setTextColor(Colour_Yellow, Colour_Black);
			printAligned(
				{{0, BootMenu::getConfirmButtonPos().bottom() + 5}, SCREEN_WIDTH, 40},
				HAlign::Center, VAlign::Center,
				F("Click confirm if you are really sure.")
			);
			printAligned(
				{{0, BootMenu::getConfirmButtonPos().bottom() + 25}, SCREEN_WIDTH, 40},
				HAlign::Center, VAlign::Center,
				F("Reboot the board to go back.")
			);
		}
	}
	else
	{
		if (m_outer.m_touch.pressed && BootMenu::getConfirmButtonPos().contains(m_outer.m_touch.pos))
		{
			gCtx.data.save();
			m_outer.changeToState(m_outer.m_states.overview);
		}
	}

}

void DisplayTFT::BootMenuState::onEnter()
{
	gScreen.setTextColor(Colour_Yellow, Colour_Black);
	printAligned(
		{{0, BootMenu::getResetButtonPos().top() - 40}, SCREEN_WIDTH, 40},
		HAlign::Center, VAlign::Center,
		F("Click to reset config")
		);

	// We don't need anything fancy for the BootMenu, so we can draw the button once and be done with it.
	TextButton btn;
	btn.init(
		0, MEDIUM_FONT, BootMenu::getResetButtonPos(),
		Colour_White, Colour_LightGrey, Colour_Black,
		"Reset");
	btn.draw();

}

void DisplayTFT::BootMenuState::onLeave()
{
}

//////////////////////////////////////////////////////////////////////////
// OverviewState
//////////////////////////////////////////////////////////////////////////

namespace { namespace Overview {

//
// Sensor data (left to the history plot)
//
#define DEFINE_SENSOR_LABEL_LINE(SENSOR_INDEX, LINE_INDEX, FLAGS) \
const LabelData sensor##SENSOR_INDEX##line##LINE_INDEX PROGMEM = \
{ \
	{ \
		LayoutHelper::getHistoryPlotRect(SENSOR_INDEX).x + LayoutHelper::getHistoryPlotRect(SENSOR_INDEX).width + 2, \
		LayoutHelper::getHistoryPlotRect(SENSOR_INDEX).y + (LINE_INDEX * (GRAPH_HEIGHT/3)),  \
		SCREEN_WIDTH - (LayoutHelper::getHistoryPlotRect(SENSOR_INDEX).x + LayoutHelper::getHistoryPlotRect(SENSOR_INDEX).width + 2), \
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

const LabelData humidityLabelData PROGMEM = \
{ \
	LayoutHelper::getStatusBarCells(STATUS_BAR_DIVISIONS-4, 4), \
	HAlign::Center, VAlign::Center, \
	SMALL_FONT, \
	HUMIDITY_LABEL_TEXT_COLOUR, GRAPH_VALUES_BKG_COLOUR, \
	WidgetFlag::EraseBkg | WidgetFlag::DrawBorder\
};

FixedLabel<> humidityLabel(&humidityLabelData, F("---.-%"));


const LabelData temperatureLabelData PROGMEM = \
{ \
	LayoutHelper::getStatusBarCells(STATUS_BAR_DIVISIONS-8, 4), \
	HAlign::Center, VAlign::Center, \
	SMALL_FONT, \
	TEMPERATURE_LABEL_TEXT_COLOUR, GRAPH_VALUES_BKG_COLOUR, \
	WidgetFlag::EraseBkg | WidgetFlag::DrawBorder\
};

FixedLabel<> temperatureLabel(&temperatureLabelData, F("---.-C"));

const LabelData runningTimeLabelData PROGMEM = \
{ \
	LayoutHelper::getStatusBarCells(STATUS_BAR_DIVISIONS-16, 8), \
	HAlign::Center, VAlign::Center, \
	SMALL_FONT, \
	RUNNINGTIME_LABEL_TEXT_COLOUR, GRAPH_VALUES_BKG_COLOUR, \
	WidgetFlag::EraseBkg | WidgetFlag::DrawBorder\
};

FixedLabel<14> runningTimeLabel(&runningTimeLabelData, F("---d--h--m--s"));

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
	m_shotConfirmationMenu.init();
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
			consumed = m_currentMenu->processTouch(m_outer.m_touch.pos);
		}
	}

	Menu* newCurrentMenu = nullptr;
	if (m_currentMenu == &m_sensorMainMenu)
	{
		// Doing this before the tick, so if we switch to the Settings menu, the main menu has a chance to be drawn as disabled
		MainMenu::ButtonId buttonId = m_sensorMainMenu.checkButtonPressed();
		if (buttonId == MainMenu::ButtonId::Settings)
		{
			CZ_LOG(logDefault, Log, F("Switching to settings"));
			m_sensorMainMenu.hide();	
			m_settingsMenu.show();
			newCurrentMenu = &m_settingsMenu;
		}
		else if (buttonId == MainMenu::ButtonId::Shot)
		{
			CZ_LOG(logDefault, Log, F("Switching to shot confirmation menu"));
			m_sensorMainMenu.hide();
			m_shotConfirmationMenu.show();
			newCurrentMenu = &m_shotConfirmationMenu;
		}
	}
	else if (m_currentMenu->checkClose())
	{
		CZ_LOG(logDefault, Log, F("Switching to main menu"));
		m_currentMenu->hide();
		m_sensorMainMenu.show();
		newCurrentMenu = &m_sensorMainMenu;
	}

	// Tick the current and new menu
	m_currentMenu->tick(deltaSeconds);
	if (newCurrentMenu)
	{
		newCurrentMenu->tick(deltaSeconds);
		m_currentMenu = newCurrentMenu;
	}

	{
		Timer::RunningTime runningTime = gTimer.getRunningTime();
		if (runningTime.seconds != m_previousRunningTimeSeconds)
		{
			m_previousRunningTimeSeconds = runningTime.seconds;
			Overview::runningTimeLabel.setText(formatString(F("%dd%dh%dm%ds"), 
				runningTime.days,
				runningTime.hours,
				runningTime.minutes,
				runningTime.seconds));
		}
	}
	
	draw();
}

void DisplayTFT::OverviewState::onEnter()
{
	m_forceRedraw = true;
	m_currentMenu = &m_sensorMainMenu;
	memset(m_sensorUpdates, 0, sizeof(m_sensorUpdates));

	m_sensorMainMenu.setForceDraw();
	m_settingsMenu.setForceDraw();
	m_shotConfirmationMenu.setForceDraw();

	gScreen.setTextColor(GRAPH_VALUES_TEXT_COLOUR, Colour_VeryDarkGrey);
	gScreen.setFont(TINY_FONT);
	for(int i=0; i<NUM_PAIRS; i++)
	{
		printAligned(
			{{0, LayoutHelper::getHistoryPlotRect(i).top()}, GROUP_NUM_WIDTH, LayoutHelper::getHistoryPlotRect(i).height},
			HAlign::Center, VAlign::Center,
			formatString(F("%d"), i+1),
			true
		);
	}
}

void DisplayTFT::OverviewState::onLeave()
{
}

void DisplayTFT::OverviewState::onEvent(const Event& evt)
{
	m_currentMenu->onEvent(evt);
	
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

		case Event::TemperatureSensorReading:
			Overview::temperatureLabel.setText(formatString(F("%2.1fC"), gCtx.data.getTemperatureReading()));
		break;

		case Event::HumiditySensorReading:
			Overview::humidityLabel.setText(formatString(F("%3.1f%%"), gCtx.data.getHumidityReading()));
		break;

		default:
		break;
	}
	
}

void DisplayTFT::OverviewState::draw()
{
	PROFILE_SCOPE(F("OverviewState::draw"));

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
				Overview::sensorLabels[i][1].setValueAndDraw(data.getCurrentValueAsPercentage(), m_forceRedraw);
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

	{
		PROFILE_SCOPE(F("statusbar"));
		Overview::temperatureLabel.draw(m_forceRedraw);
		Overview::humidityLabel.draw(m_forceRedraw);
		Overview::runningTimeLabel.draw(m_forceRedraw);
	}

	m_forceRedraw = false;
}

//////////////////////////////////////////////////////////////////////////
// DisplayTFT
//////////////////////////////////////////////////////////////////////////

DisplayTFT::DisplayTFT()
	: m_states(*this)
{
}

void DisplayTFT::begin()
{
	m_states.initialize.init();
	m_states.intro.init();
	m_states.bootMenu.init();
	m_states.overview.init();
	changeToState(m_states.initialize);
}

float DisplayTFT::tick(float deltaSeconds)
{
	PROFILE_SCOPE(F("DisplayTFT::tick"));

	m_timeInState += deltaSeconds;

	updateTouch(deltaSeconds);

	//CZ_LOG(logDefault, Log, F("DisplayTFT::%s: state=%s, timeInState = %d"), __FUNCTION__, ms_stateNames[(int)m_state], (int)m_timeInState);
	m_state->tick(deltaSeconds);

	return 1.0f / 30.0f;
}

void DisplayTFT::updateTouch(float deltaSeconds)
{
	TouchPoint p = gTs.getPoint();

	if (p.z > TS_MIN_PRESSURE)
	{
		//CZ_LOG(logDefault, Log, F("Touch=(%3d,%3d,%3d)"), p.x, p.y, p.z);
	}

	m_touch.pressed = false;
	// If we are not touching right now, but were in the previous call, that means we have a press event
	if (p.z < TS_MIN_PRESSURE && m_touch.tmp.z >= TS_MIN_PRESSURE)
	{
		m_touch.secondsSinceLastTouch = 0;
		if (m_touch.sleeping)
		{
			CZ_LOG(logDefault, Log, F("Waking up"));
			m_touch.sleeping = false;
			gScreen.setBacklightBrightness(SCREEN_DEFAULT_BRIGHTNESS);
		}
		else
		{
			m_touch.pos = {p.x, p.y};
			m_touch.pressed = true;
			CZ_LOG(logDefault, Verbose, F("Press=(%3d,%3d)"), m_touch.pos.x, m_touch.pos.y);
		}
	}
	else
	{
		if (m_touch.sleeping)
		{
			if (m_touch.currentBrightness>0)
			{
				m_touch.currentBrightness -= deltaSeconds * SCREEN_OFF_DIM_SPEED;
				gScreen.setBacklightBrightness((uint8_t)cz::clamp(m_touch.currentBrightness, 0.0f, 100.0f));
			}
		}
		else
		{
			m_touch.secondsSinceLastTouch += deltaSeconds;
			if (m_touch.secondsSinceLastTouch > SCREEN_OFF_TIMEOUT)
			{
				CZ_LOG(logDefault, Log, F("Starting sleep"));
				m_touch.sleeping = true;
				m_touch.currentBrightness = SCREEN_DEFAULT_BRIGHTNESS;
			}
		}
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
