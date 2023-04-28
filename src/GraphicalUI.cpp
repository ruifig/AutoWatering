#include "GraphicalUI.h"
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
extern XPT2046 gTs;

extern Timer gTimer;

using namespace gfx;

//////////////////////////////////////////////////////////////////////////
// InitializeState
//////////////////////////////////////////////////////////////////////////

void GraphicalUI::InitializeState::tick(float deltaSeconds)
{
	m_outer.changeToState(m_outer.m_states.intro);
}

void GraphicalUI::InitializeState::onEnter()
{
	gScreen.fillScreen(Colour_Black);
}

void GraphicalUI::InitializeState::onLeave()
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
		{10, 10, AW_SCREEN_WIDTH - 20, AW_SCREEN_HEIGHT - 20 }, // box
		HAlign::Left, VAlign::Top,
		LARGE_FONT,
		AW_INTRO_TEXT_COLOUR,
		AW_SCREEN_BKG_COLOUR,
		WidgetFlag::None
	},
	(const __FlashStringHelper*)introLabel1_value_P
};

const char introLabel2_value_P[] PROGMEM = "By";
const StaticLabelData introLabel2_P PROGMEM =
{
	{
		{10, 10, AW_SCREEN_WIDTH - 20, AW_SCREEN_HEIGHT - 20 }, // box
		HAlign::Center, VAlign::Center,
		LARGE_FONT,
		AW_INTRO_TEXT_COLOUR,
		AW_SCREEN_BKG_COLOUR,
		WidgetFlag::None
	},
	(const __FlashStringHelper*)introLabel2_value_P
};

const char introLabel3_value_P[] PROGMEM = "Rui Figueira";
const StaticLabelData introLabel3_P PROGMEM =
{
	{
		{10, 10, AW_SCREEN_WIDTH - 20, AW_SCREEN_HEIGHT - 20 }, // box
		HAlign::Right, VAlign::Bottom,
		LARGE_FONT,
		AW_INTRO_TEXT_COLOUR,
		AW_SCREEN_BKG_COLOUR,
		WidgetFlag::None
	},
	(const __FlashStringHelper*)introLabel3_value_P
};

} } // namespace Intro


void GraphicalUI::IntroState::tick(float deltaSeconds)
{
	if (m_outer.m_timeInState >= AW_INTRO_DURATION)
	{
		m_outer.changeToState(m_outer.m_states.bootMenu);
	}
}

void GraphicalUI::IntroState::onEnter()
{
	StaticLabel(&Intro::introLabel1_P).draw();
	StaticLabel(&Intro::introLabel2_P).draw();
	StaticLabel(&Intro::introLabel3_P).draw();
}

void GraphicalUI::IntroState::onLeave()
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
		return { {(AW_SCREEN_WIDTH-width)/2, (AW_SCREEN_HEIGHT-height)/2}, width, height};
	}
	constexpr Rect getConfirmButtonPos()
	{
		constexpr int width = 200;
		constexpr int height = 40;
		return { {(AW_SCREEN_WIDTH-width)/2, 10}, width, height};
	}

} }

void GraphicalUI::BootMenuState::init()
{
}

void GraphicalUI::BootMenuState::tick(float deltaSeconds)
{
	m_defaultConfigCountdown -= deltaSeconds;

	if (m_waitingResetConfirmation == false)
	{
		gScreen.setTextColor(Colour_Yellow, Colour_Black);
		printAligned(
			{{0, BootMenu::getResetButtonPos().bottom() + 10}, AW_SCREEN_WIDTH, 40},
			HAlign::Center, VAlign::Center,
			F(" Loading saved config in... ")
		);
		printAligned(
			{{0, BootMenu::getResetButtonPos().bottom() + 40}, AW_SCREEN_WIDTH, 40},
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
				{{0, BootMenu::getConfirmButtonPos().bottom() + 5}, AW_SCREEN_WIDTH, 40},
				HAlign::Center, VAlign::Center,
				F("Click confirm if you are really sure.")
			);
			printAligned(
				{{0, BootMenu::getConfirmButtonPos().bottom() + 25}, AW_SCREEN_WIDTH, 40},
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

void GraphicalUI::BootMenuState::onEnter()
{
	gScreen.setTextColor(Colour_Yellow, Colour_Black);
	printAligned(
		{{0, BootMenu::getResetButtonPos().top() - 40}, AW_SCREEN_WIDTH, 40},
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

void GraphicalUI::BootMenuState::onLeave()
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
		LayoutHelper::getHistoryPlotRect(SENSOR_INDEX).y + (LINE_INDEX * (AW_GRAPH_HEIGHT/3)),  \
		AW_SCREEN_WIDTH - (LayoutHelper::getHistoryPlotRect(SENSOR_INDEX).x + LayoutHelper::getHistoryPlotRect(SENSOR_INDEX).width + 2), \
		AW_GRAPH_HEIGHT/3, \
	}, \
	HAlign::Center, VAlign::Center, \
	TINY_FONT, \
	AW_GRAPH_VALUES_TEXT_COLOUR, AW_GRAPH_VALUES_BKG_COLOUR, \
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

NumLabel<true> sensorLabels[AW_VISIBLE_NUM_PAIRS][3] =
{
	{
		{ sensor0line0 },
		{ sensor0line1 },
		{ sensor0line2 }
	}
#if AW_VISIBLE_NUM_PAIRS>1
	,
	{
		{ sensor1line0 },
		{ sensor1line1 },
		{ sensor1line2 }
	}
#endif
#if AW_VISIBLE_NUM_PAIRS>2
	,
	{
		{ sensor2line0 },
		{ sensor2line1 },
		{ sensor2line2 }
	}
#endif
#if AW_VISIBLE_NUM_PAIRS>3
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
	LayoutHelper::getStatusBarCells(STATUS_BAR_DIVISIONS-3, 3), \
	HAlign::Center, VAlign::Center, \
	SMALL_FONT, \
	AW_HUMIDITY_LABEL_TEXT_COLOUR, AW_GRAPH_VALUES_BKG_COLOUR, \
	WidgetFlag::EraseBkg | WidgetFlag::DrawBorder\
};

FixedLabel<> humidityLabel(&humidityLabelData, F("---.-%"));


const LabelData temperatureLabelData PROGMEM = \
{ \
	LayoutHelper::getStatusBarCells(STATUS_BAR_DIVISIONS-6, 3), \
	HAlign::Center, VAlign::Center, \
	SMALL_FONT, \
	AW_TEMPERATURE_LABEL_TEXT_COLOUR, AW_GRAPH_VALUES_BKG_COLOUR, \
	WidgetFlag::EraseBkg | WidgetFlag::DrawBorder\
};

FixedLabel<> temperatureLabel(&temperatureLabelData, F("---.-C"));

const LabelData runningTimeLabelData PROGMEM = \
{ \
	LayoutHelper::getStatusBarCells(STATUS_BAR_DIVISIONS-14, 8), \
	HAlign::Center, VAlign::Center, \
	SMALL_FONT, \
	AW_RUNNINGTIME_LABEL_TEXT_COLOUR, AW_GRAPH_VALUES_BKG_COLOUR, \
	WidgetFlag::EraseBkg | WidgetFlag::DrawBorder\
};

FixedLabel<14> runningTimeLabel(&runningTimeLabelData, F("---d--h--m--s"));

const LabelData batteryLabelData PROGMEM = \
{ \
	LayoutHelper::getStatusBarCells(0, 6), \
	HAlign::Center, VAlign::Center, \
	SMALL_FONT, \
	AW_BATTERYLEVEL_LABEL_TEXT_COLOUR, AW_GRAPH_VALUES_BKG_COLOUR, \
	WidgetFlag::EraseBkg | WidgetFlag::DrawBorder \
};

FixedLabel<12> batteryLabel(&batteryLabelData, F("---% -.---v"));

} } // namespace Overview

GraphicalUI::OverviewState::OverviewState(GraphicalUI& outer)
	: DisplayState(outer)
{
}

void GraphicalUI::OverviewState::init()
{
	m_topSlotPairIndex = 0;
	scrollSlots(0);

	m_sensorMainMenu.init();
	m_settingsMenu.init();
	m_shotConfirmationMenu.init();
}

void GraphicalUI::OverviewState::tick(float deltaSeconds)
{
	// process touch
	if (m_outer.m_touch.pressed)
	{
		bool consumed = false;

		for(Group& group : m_groups)
		{
			consumed = group.graph.processTouch(m_outer.m_touch.pos);
			if (consumed)
			{
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
		else if (buttonId == MainMenu::ButtonId::Up)
		{
			CZ_LOG(logDefault, Log, F("Scrolling up"));
			scrollSlots(-1);
		}
		else if (buttonId == MainMenu::ButtonId::Down)
		{
			CZ_LOG(logDefault, Log, F("Scrolling down"));
			scrollSlots(1);
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

GraphicalUI::OverviewState::Group* GraphicalUI::OverviewState::tryGetGroupByPairIndex(int8_t pairIndex)
{
	int8_t screenSlot = pairIndex - m_topSlotPairIndex;
	if (screenSlot>=0 && screenSlot<AW_VISIBLE_NUM_PAIRS)
	{
		return &m_groups[screenSlot];
	}
	else
	{
		return nullptr;
	}
}

GraphicalUI::OverviewState::Group& GraphicalUI::OverviewState::getGroupByPairIndex(int8_t pairIndex)
{
	Group* g = tryGetGroupByPairIndex(pairIndex);
	CZ_ASSERT(g!=nullptr);
	return *g;
}

void GraphicalUI::OverviewState::drawGroupNumbers()
{
	gScreen.setTextColor(AW_GRAPH_VALUES_TEXT_COLOUR, Colour_VeryDarkGrey);
	gScreen.setFont(TINY_FONT);
	for(int i=0; i<AW_VISIBLE_NUM_PAIRS; i++)
	{
		int pairIndex = i + m_topSlotPairIndex;
		if (pairIndex < MAX_NUM_PAIRS)
		{
			printAligned(
				{{0, LayoutHelper::getHistoryPlotRect(i).top()}, AW_GROUP_NUM_WIDTH, LayoutHelper::getHistoryPlotRect(i).height},
				HAlign::Center, VAlign::Center,
				formatString(F("%d"), pairIndex + 1),
				true
			);
		}
	}
}

void GraphicalUI::OverviewState::onEnter()
{
	m_forceRedraw = true;
	m_currentMenu = &m_sensorMainMenu;

	for(Group& group : m_groups)
	{
		group.sensorUpdates = false;
	}

	m_sensorMainMenu.setForceDraw();
	m_settingsMenu.setForceDraw();
	m_shotConfirmationMenu.setForceDraw();

	drawGroupNumbers();
}

void GraphicalUI::OverviewState::onLeave()
{
}


void GraphicalUI::OverviewState::scrollSlots(int inc)
{
	m_topSlotPairIndex = clamp(m_topSlotPairIndex+inc, 0, MAX_NUM_PAIRS - AW_VISIBLE_NUM_PAIRS);

	// If for some reason we can't unselect the selected group, then we don't perform any scrolling
	if (!gCtx.data.trySetSelectedGroup(-1))
	{
		return;
	}

	drawGroupNumbers();

	for(int idx = 0; idx<AW_VISIBLE_NUM_PAIRS; idx++)
	{
		int pairIndex = idx + m_topSlotPairIndex;
		// Pass pairIndex as -1 if it's an empty screen slot
		m_groups[idx].graph.setAssociation(idx, (pairIndex<MAX_NUM_PAIRS) ? pairIndex : -1);
		m_groups[idx].sensorUpdates = true;
		Overview::sensorLabels[idx][0].clearValue();
		Overview::sensorLabels[idx][1].clearValue();
		Overview::sensorLabels[idx][2].clearValue();
	}

}

void GraphicalUI::OverviewState::onEvent(const Event& evt)
{
	m_currentMenu->onEvent(evt);
	
	for(Group& g : m_groups)
	{
		g.graph.onEvent(evt);
	}

	switch(evt.type)
	{
		case Event::ConfigLoad:
		{
			int8_t idx = static_cast<const ConfigLoadEvent&>(evt).group;
			if (idx == -1)
			{
				m_forceRedraw = true;
			}
			else
			{
				if (Group* group = tryGetGroupByPairIndex(idx))
				{
					// Force an update to the group labels
					group->sensorUpdates = true;
				}
			}
		}
		break;

		case Event::GroupOnOff:
		{
			const GroupOnOffEvent& e = static_cast<const GroupOnOffEvent&>(evt);
			// Force an update to the group labels;
			if (Group* group = tryGetGroupByPairIndex(e.index))
			{
				group->sensorUpdates = true;
			}
		}
		break;

		case Event::SoilMoistureSensorReading:
		{
			auto idx = static_cast<const SoilMoistureSensorReadingEvent&>(evt).index;
			if (Group* group = tryGetGroupByPairIndex(idx))
			{
				group->sensorUpdates = true;
			}
		}
		break;

		case Event::TemperatureSensorReading:
			Overview::temperatureLabel.setText(formatString(F("%2.1fC"), gCtx.data.getTemperatureReading()));
		break;

		case Event::HumiditySensorReading:
			Overview::humidityLabel.setText(formatString(F("%3.1f%%"), gCtx.data.getHumidityReading()));
		break;

		case Event::BatteryLifeReading:
		{
			const BatteryLifeReadingEvent& e = static_cast<const BatteryLifeReadingEvent&>(evt);
			Overview::batteryLabel.setText(formatString(F("%d%% %1.3fv"), e.percentage, e.voltage));
		}
		break;

		default:
		break;
	}
	
}

void GraphicalUI::OverviewState::draw()
{
	PROFILE_SCOPE(F("OverviewState::draw"));

	for(int i=0; i<AW_VISIBLE_NUM_PAIRS; i++)
	{
		PROFILE_SCOPE(F("groupDrawing"));

		m_groups[i].graph.draw(m_forceRedraw);
		GroupData& data = gCtx.data.getGroupData(i+m_topSlotPairIndex);

		//
		// Draw values
		//
		if (m_groups[i].sensorUpdates || m_forceRedraw)
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
			m_groups[i].sensorUpdates = false;
		}
	}

	{
		PROFILE_SCOPE(F("statusbar"));
		Overview::temperatureLabel.draw(m_forceRedraw);
		Overview::humidityLabel.draw(m_forceRedraw);
		Overview::runningTimeLabel.draw(m_forceRedraw);
		Overview::batteryLabel.draw(m_forceRedraw);
	}

	m_forceRedraw = false;
}

//////////////////////////////////////////////////////////////////////////
// GraphicalUI
//////////////////////////////////////////////////////////////////////////

GraphicalUI::GraphicalUI()
	: m_states(*this)
{
}

bool GraphicalUI::initImpl()
{
	m_states.initialize.init();
	m_states.intro.init();
	m_states.bootMenu.init();
	m_states.overview.init();
	changeToState(m_states.initialize);
	return true;
}

float GraphicalUI::tick(float deltaSeconds)
{
	PROFILE_SCOPE(F("GraphicalUI::tick"));

	m_timeInState += deltaSeconds;

	updateTouch(deltaSeconds);

	//CZ_LOG(logDefault, Log, F("GraphicalUI::%s: state=%s, timeInState = %d"), __FUNCTION__, ms_stateNames[(int)m_state], (int)m_timeInState);
	m_state->tick(deltaSeconds);

	return m_touch.sleeping ? 0.250f : (1.0f / 30.0f);
}

void GraphicalUI::updateTouch(float deltaSeconds)
{
	gTs.updateState();

	TouchState touchState = gTs.getState();
	m_touch.pressed = false;
	TouchPoint p = gTs.getPoint();

	if (touchState == TouchState::Released)
	{
		m_touch.secondsSinceLastTouch = 0;
		// If we are currently sleeping, we wake up the screen and ignore this Release
		if (m_touch.sleeping)
		{
			CZ_LOG(logDefault, Log, F("Waking up"));
			m_touch.sleeping = false;
			gScreen.setBacklightBrightness(AW_SCREEN_DEFAULT_BRIGHTNESS);
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
				m_touch.currentBrightness -= deltaSeconds * AW_SCREEN_OFF_DIM_SPEED;
				gScreen.setBacklightBrightness((uint8_t)cz::clamp(m_touch.currentBrightness, 0.0f, 100.0f));
			}
		}
		else
		{
			m_touch.secondsSinceLastTouch += deltaSeconds;
			if ((AW_SCREEN_OFF_TIMEOUT!=0) && (m_touch.secondsSinceLastTouch > AW_SCREEN_OFF_TIMEOUT))
			{
				CZ_LOG(logDefault, Log, F("Turning off screen"));
				m_touch.sleeping = true;
				m_touch.currentBrightness = AW_SCREEN_DEFAULT_BRIGHTNESS;
			}
		}
	}
}
	
void GraphicalUI::onEvent(const Event& evt)
{
	m_state->onEvent(evt);	
}

bool GraphicalUI::processCommand(const Command& cmd)
{
	if (cmd.is("scroll"))
	{
		int inc;
		if (cmd.parseParams(inc))
		{
			scrollSlots(inc);
			return true;
		}
	}

	return false;
}

void GraphicalUI::scrollSlots(int inc)
{
	m_states.overview.scrollSlots(inc);
}

void GraphicalUI::changeToState(DisplayState& newState)
{
	// NOTE: We need to take into account m_state will not be yet set if we are starting up
#if CZ_LOG_ENABLED
	CZ_LOG(logDefault, Log, F("GraphicalUI::%s %ssec %s->%s")
		, __FUNCTION__
		, *FloatToString(m_timeInState)
		, m_state ? m_state->getName() : "NONE"
		, newState.getName());
#endif

	if (m_state)
	{
		m_state->onLeave();
	}

	gScreen.fillScreen(AW_SCREEN_BKG_COLOUR);
	m_state = &newState;
    m_timeInState = 0.0f;
	m_state->onEnter();
}

#if AW_GRAPHICALUI_ENABLED
	GraphicalUI gGraphicalUI;
#endif

} // namespace cz
