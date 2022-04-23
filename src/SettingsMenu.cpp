#include "SettingsMenu.h"
#include "Icons.h"
#include "DisplayCommon.h"

extern uint32_t gTickCount;

namespace cz
{

using namespace gfx;

namespace
{
	#define DEFINE_SENSOR_CALIBRATION_LABEL_LINE(LINE_INDEX, FLAGS) \
	const LabelData sensorCalibrationSettings##LINE_INDEX PROGMEM = \
	{ \
		{LayoutHelper::getMenuButtonPos(1, 1).x, LayoutHelper::getMenuButtonPos(1,1).y + (LINE_INDEX * (GRAPH_HEIGHT/3)), 32, GRAPH_HEIGHT/3}, \
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
		{LayoutHelper::getMenuButtonPos(1+COL, 1).x, LayoutHelper::getMenuButtonPos(1+COL,1).y + (CELL_LINE * (GRAPH_HEIGHT/2)), 32, GRAPH_HEIGHT/2}, \
		HAlign::Center, VAlign::Center, \
		TINY_FONT, \
		GRAPH_VALUES_TEXT_COLOUR, GRAPH_VALUES_BKG_COLOUR, \
		WidgetFlag::EraseBkg | FLAGS \
	};

	DEFINE_LABEL_LINE2(1, samplingInterval, 0, WidgetFlag::None)
	DEFINE_LABEL_LINE2(1, samplingInterval, 1, WidgetFlag::None)
	DEFINE_LABEL_LINE2(2, shotDuration, 0, WidgetFlag::None)
	DEFINE_LABEL_LINE2(2, shotDuration, 1, WidgetFlag::None)
}

SettingsMenu::SettingsMenu()
	: m_calibrationLabels
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

#define CALIBRATE_BTN_COL 1
#define SENSORINTERVAL_BTN_COL 2
#define SHOTDURATION_BTN_COL 3

void SettingsMenu::init()
{
	auto initButton = [this](ButtonId id, auto&&... params)
	{
		m_buttons[(int)id].init((int)id, std::forward<decltype(params)>(params)...);
		m_buttons[(int)id].setClearWhenHidden(true);
	};

	// First line
	initButton(ButtonId::CloseAndSave, LayoutHelper::getMenuButtonPos(0,0), SCREEN_BKG_COLOUR, img_Save);
	initButton(ButtonId::Calibrate, LayoutHelper::getMenuButtonPos(CALIBRATE_BTN_COL,0), SCREEN_BKG_COLOUR, img_Ruler);
	initButton(ButtonId::SensorInterval, LayoutHelper::getMenuButtonPos(SENSORINTERVAL_BTN_COL,0), SCREEN_BKG_COLOUR, img_SetSensorInterval);
	initButton(ButtonId::ShotDuration, LayoutHelper::getMenuButtonPos(SHOTDURATION_BTN_COL,0), SCREEN_BKG_COLOUR, img_SetWateringDuration);
	// Leaving one empty grid space intentionally, so the CloseAndIgnore button is spaced away from the others
	initButton(ButtonId::CloseAndIgnore, LayoutHelper::getMenuButtonPos(5,0), SCREEN_BKG_COLOUR, img_Close);

	// Second line
	initButton(ButtonId::SetGroupThreshold, LayoutHelper::getMenuButtonPos(CALIBRATE_BTN_COL+1,1), SCREEN_BKG_COLOUR, img_SetThreshold);
	initButton(ButtonId::Minus, LayoutHelper::getMenuButtonPos(0,1), SCREEN_BKG_COLOUR, img_Remove);
	initButton(ButtonId::Plus, LayoutHelper::getMenuButtonPos(0,1), SCREEN_BKG_COLOUR, img_Add);

	//hide();
}

void SettingsMenu::tick(float deltaSeconds)
{
	draw();
}

void SettingsMenu::setState(State state)
{
	m_state = state;
	clearEntireArea();

	// First set all to hiden/disabled, as we'll be enabling just the ones we need
	setButtonRange(ButtonId::First, ButtonId::Max, false, false);

	// 1st line - These are always enabled visible regardless of what menu we are in, although they might be in a disabled state
	setButtonRange(ButtonId::Calibrate, ButtonId::CloseAndIgnore, true, true);

	bool showMinusPlus = (m_state==State::SettingSensorInterval || m_state==State::SettingShotDuration);
	setButton(ButtonId::SetGroupThreshold, state==State::CalibratingSensor, state==State::CalibratingSensor);
	setButton(ButtonId::Minus, showMinusPlus, showMinusPlus);
	setButton(ButtonId::Plus, showMinusPlus, showMinusPlus);

	CZ_LOG(logDefault, Log, F("%s(%d):TickCount=%u"), __FUNCTION__, (int)state, gTickCount);

	if (state==State::Main || state==State::CalibratingSensor)
	{
		setSensorLabels(true);
	}
	else
	{
		for(auto&& l : m_calibrationLabels)
		{
			l.clearValueAndDraw();
			l.setDirty(true);
		}

	}

	//
	// Sampling interval labels
	//
	if (state==State::Main || state==State::SettingSensorInterval)
	{
		changeSamplingInterval(0);
		m_samplingIntervalLabels[1].setText("min");
	}
	else
	{
		m_samplingIntervalLabels[0].clearValueAndDraw();
		m_samplingIntervalLabels[1].clearValueAndDraw();
	}

	//
	// Shot duration labels
	//
	if (state==State::Main || state==State::SettingShotDuration)
	{
		changeShotDuration(0);
		m_shotDurationLabels[1].setText("sec");
	}
	else
	{
		m_shotDurationLabels[0].clearValueAndDraw();
		m_shotDurationLabels[1].clearValueAndDraw();
	}

	// Draw buttons on the first line as disabled when required
	m_buttons[(int)ButtonId::Calibrate].setEnabled(state==State::Main || state==State::CalibratingSensor);
	m_buttons[(int)ButtonId::SensorInterval].setEnabled(state==State::Main || state==State::SettingSensorInterval);
	m_buttons[(int)ButtonId::ShotDuration].setEnabled(state==State::Main || state==State::SettingShotDuration);

	if (m_state==State::CalibratingSensor)
	{
	}
	else if (state==State::SettingSensorInterval)
	{
		m_buttons[(int)ButtonId::Minus].move(LayoutHelper::getMenuButtonPos(SENSORINTERVAL_BTN_COL-1, 1), true);
		m_buttons[(int)ButtonId::Plus ].move(LayoutHelper::getMenuButtonPos(SENSORINTERVAL_BTN_COL+1, 1), true);
	}
	else if (state==State::SettingShotDuration)
	{
		m_buttons[(int)ButtonId::Minus].move(LayoutHelper::getMenuButtonPos(SHOTDURATION_BTN_COL-1, 1), true);
		m_buttons[(int)ButtonId::Plus ].move(LayoutHelper::getMenuButtonPos(SHOTDURATION_BTN_COL+1, 1), true);
	}
}

bool SettingsMenu::processTouch(const Pos& pos)
{
	CZ_ASSERT(gCtx.data.hasGroupSelected());

	auto checkButtonPressed = [&](ButtonId id) -> bool
	{
		ImageButton& btn = m_buttons[(int)id];
		if (btn.canAcceptInput() && btn.contains(pos))
		{
			m_pressedId = id;
			return true;
		}
		else
		{
			return false;
		}
	};

	if (
		checkButtonPressed(ButtonId::CloseAndSave) ||
		checkButtonPressed(ButtonId::CloseAndIgnore))
	{
		return true;
	}
	
	if (checkButtonPressed(ButtonId::Calibrate))
	{
		// If we are already in the sensor calibration menu, then close that one and go back to the Main
		setState(m_state==State::CalibratingSensor ? State::Main : State::CalibratingSensor);
		return true;
	}
	else if(checkButtonPressed(ButtonId::SensorInterval))
	{
		setState(m_state==State::SettingSensorInterval ? State::Main : State::SettingSensorInterval);
		return true;
	}
	else if(checkButtonPressed(ButtonId::ShotDuration))
	{
		setState(m_state==State::SettingShotDuration ? State::Main : State::SettingShotDuration);
		return true;
	}

	return false;
}

void SettingsMenu::onEvent(const Event& evt)
{
	switch(evt.type)
	{
		case Event::SoilMoistureSensorCalibrationReading:
		{
			const SoilMoistureSensorCalibrationReadingEvent& e = static_cast<const SoilMoistureSensorCalibrationReadingEvent&>(evt);
			if (e.index==gCtx.data.getSelectedGroupIndex() && (m_state==State::Main || m_state==State::CalibratingSensor) && e.reading.isValid())
			{
				// We only adjust the sensor range (air/water values) if we are in the sensor calibration menu
				m_dummyCfg.setSensorValue(e.reading.meanValue, m_state==State::CalibratingSensor ? true : false);
				setSensorLabels();
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
			for(auto&& l : m_calibrationLabels)
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
	Menu::show();

	GroupData* data = gCtx.data.getSelectedGroup();
	m_dummyCfg = data->copyConfig();

	CZ_LOG(logDefault, Log, F("%s:TickCount=%u"), __FUNCTION__, gTickCount);

	data->setInConfigMenu(true);
	setState(State::Main);
}

void SettingsMenu::setSensorLabels(bool forceDirty)
{
	CZ_LOG(logDefault, Log, F("%s:TickCount=%u"), __FUNCTION__, gTickCount);
	m_calibrationLabels[0].setValue(m_dummyCfg.waterValue);
	m_calibrationLabels[1].setValue(m_dummyCfg.getThresholdValueAsPercentage());
	m_calibrationLabels[2].setValue(m_dummyCfg.airValue);

	if (forceDirty)
	{
		for(auto&& l : m_calibrationLabels)
		{
			l.setDirty(true);
		}
	}
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
	setButtonRange(ButtonId::First, ButtonId::Max, false, false);
	GroupData* data = gCtx.data.getSelectedGroup();
	data->setInConfigMenu(false);

	Menu::hide();
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

} // namespace cz
