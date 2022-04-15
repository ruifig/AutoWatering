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
	else // We are 
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

// Make sure we have enough space at the bottom for the menu
static_assert((SCREEN_HEIGHT - (getHistoryPlotRect(3).y + getHistoryPlotRect(3).height)) > 64, "Need enough space left at the bottom for the menu (64 pixels high)");

constexpr Pos getMenuButtonPos(uint8_t col, uint8_t row)
{
	Pos pos{0};
	pos.x = (32 + 3) * col;
	pos.y = getHistoryPlotRect(3).y + getHistoryPlotRect(3).height + 3 + (32 + 3) * row;
	return pos;
}

#define STATUS_BAR_DIVISIONS 20
constexpr Rect getStatusBarPos()
{
	Rect rect { getMenuButtonPos(0,2), {SCREEN_WIDTH, SCREEN_HEIGHT} };
	return rect;
}

constexpr Rect getStatusBarCells(int startCell, int numCells)
{
	Rect statusBar = getStatusBarPos();
	return Rect(
		statusBar.x + startCell * (statusBar.width / STATUS_BAR_DIVISIONS), statusBar.y,
		numCells * (statusBar.width / STATUS_BAR_DIVISIONS), statusBar.height
	);
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

const LabelData humidityLabelData PROGMEM = \
{ \
	getStatusBarCells(STATUS_BAR_DIVISIONS-4, 4), \
	HAlign::Center, VAlign::Center, \
	SMALL_FONT, \
	HUMIDITY_LABEL_TEXT_COLOUR, GRAPH_VALUES_BKG_COLOUR, \
	WidgetFlag::EraseBkg | WidgetFlag::DrawBorder\
};

FixedLabel<> humidityLabel(&humidityLabelData, F("---.-%"));


const LabelData temperatureLabelData PROGMEM = \
{ \
	getStatusBarCells(STATUS_BAR_DIVISIONS-8, 4), \
	HAlign::Center, VAlign::Center, \
	SMALL_FONT, \
	TEMPERATURE_LABEL_TEXT_COLOUR, GRAPH_VALUES_BKG_COLOUR, \
	WidgetFlag::EraseBkg | WidgetFlag::DrawBorder\
};

FixedLabel<> temperatureLabel(&temperatureLabelData, F("---.-C"));

const LabelData runningTimeLabelData PROGMEM = \
{ \
	getStatusBarCells(STATUS_BAR_DIVISIONS-16, 8), \
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
	m_inSettingsMenu = false;
	memset(m_sensorUpdates, 0, sizeof(m_sensorUpdates));
	m_sensorMainMenu.setForceDraw();
	m_settingsMenu.setForceDraw();

	gScreen.setTextColor(GRAPH_VALUES_TEXT_COLOUR, Colour_VeryDarkGrey);
	gScreen.setFont(TINY_FONT);
	for(int i=0; i<NUM_PAIRS; i++)
	{
		printAligned(
			{{0, getHistoryPlotRect(i).top()}, GROUP_NUM_WIDTH, getHistoryPlotRect(i).height},
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

	{
		PROFILE_SCOPE(F("statusbar"));
		Overview::temperatureLabel.draw(m_forceRedraw);
		Overview::humidityLabel.draw(m_forceRedraw);
		Overview::runningTimeLabel.draw(m_forceRedraw);
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

// Defines the label data for menus, where a single cell can take 2 lines
#define DEFINE_LABEL_LINE2(COL, name, CELL_LINE, FLAGS) \
const LabelData labelData_##name##_##CELL_LINE PROGMEM = \
{ \
	{Overview::getMenuButtonPos(1+COL, 1).x, Overview::getMenuButtonPos(1+COL,1).y + (CELL_LINE * (GRAPH_HEIGHT/2)), 32, GRAPH_HEIGHT/2}, \
	HAlign::Center, VAlign::Center, \
	TINY_FONT, \
	GRAPH_VALUES_TEXT_COLOUR, GRAPH_VALUES_BKG_COLOUR, \
	WidgetFlag::EraseBkg | FLAGS \
};

DEFINE_LABEL_LINE2(1, samplingInterval, 0, WidgetFlag::None)
DEFINE_LABEL_LINE2(1, samplingInterval, 1, WidgetFlag::None)
DEFINE_LABEL_LINE2(2, shotDuration, 0, WidgetFlag::None)
DEFINE_LABEL_LINE2(2, shotDuration, 1, WidgetFlag::None)

SettingsMenu::SettingsMenu()
	: m_sensorLabels
		{
			{ sensorCalibrationSettings0},
			{ sensorCalibrationSettings1},
			{ sensorCalibrationSettings2},
		}
	, m_samplingIntervalLabels
		{
			{ &labelData_samplingInterval_0 },
			{ &labelData_samplingInterval_1 },
		}
	, m_shotDurationLabels
		{
			{ &labelData_shotDuration_0 },
			{ &labelData_shotDuration_1 },
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
	switch(evt.type)
	{
		case Event::SoilMoistureSensorCalibrationReading:
		{
			const SoilMoistureSensorCalibrationReadingEvent& e = static_cast<const SoilMoistureSensorCalibrationReadingEvent&>(evt);
			if (e.index==gCtx.data.getSelectedGroupIndex() && (m_state==State::Main || m_state==State::CalibratingSensor) && e.reading.isValid())
			{
				m_dummyCfg.setSensorValue(e.reading.meanValue, true);
				m_sensorLabels[0].setValue(m_dummyCfg.waterValue);
				m_sensorLabels[1].setValue(m_dummyCfg.getPercentageValue());
				m_sensorLabels[2].setValue(m_dummyCfg.airValue);
			}
		}
		break;

		default:
		break;
	}
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
			for(auto&& l : m_sensorLabels)
				l.draw();
		}

		if (m_state==State::Main || m_state==State::SettingSensorInterval)
		{
			for(auto&& l : m_samplingIntervalLabels)
				l.draw();
		}

		if (m_state==State::Main || m_state==State::SettingShotDuration)
		{
			for(auto&& l : m_shotDurationLabels)
				l.draw();
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
	GroupData* data = gCtx.data.getSelectedGroup();
	m_dummyCfg = data->copyConfig();

	// CloseAndSave is hidden until we actually enter a proper settings changing menu
	setButton(ButtonId::CloseAndSave, false, false);
	setButtonRange(ButtonId::Calibrate, ButtonId::CloseAndIgnore, true, true);

	setButtonRange(ButtonId::SetGroupThreshold, ButtonId::Plus, false, false);

	for(gfx::NumLabel<true>& label : m_sensorLabels)
	{
		label.clearValue();
	}

	changeSamplingInterval(0);
	m_samplingIntervalLabels[1].setText("min");
	// Forcibly mark it for redraw, otherwise it won't show up if we close and reopen the settings menu
	for(auto&& l : m_samplingIntervalLabels)
		l.setDirty(true);

	changeShotDuration(0);
	m_shotDurationLabels[1].setText("sec");
	// Forcibly mark it for redraw, otherwise it won't show up if we close and reopen the settings menu
	for(auto&& l : m_shotDurationLabels)
		l.setDirty(true);

	data->setInConfigMenu(true);
}

void SettingsMenu::changeSamplingInterval(int direction)
{
	m_dummyCfg.setSamplingInterval(m_dummyCfg.samplingInterval + direction*60);
	m_samplingIntervalLabels[0].setText(*IntToString(static_cast<int>(m_dummyCfg.getSamplingIntervalInMinutes())));
}

void SettingsMenu::changeShotDuration(int direction)
{
	m_dummyCfg.setShotDuration(m_dummyCfg.shotDuration + direction);
	m_shotDurationLabels[0].setText(*IntToString(static_cast<int>(m_dummyCfg.shotDuration)));
}

void SettingsMenu::hide()
{
	setButtonRange(ButtonId::CloseAndSave, ButtonId::Plus, false, false);
	GroupData* data = gCtx.data.getSelectedGroup();
	data->setInConfigMenu(false);
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

	updateTouch();

	//CZ_LOG(logDefault, Log, F("DisplayTFT::%s: state=%s, timeInState = %d"), __FUNCTION__, ms_stateNames[(int)m_state], (int)m_timeInState);
	m_state->tick(deltaSeconds);

	return 1.0f / 30.0f;
}

void DisplayTFT::updateTouch()
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
		m_touch.pos = {p.x, p.y};
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
