#include "SettingsMenu.h"
#include "Icons.h"
#include "DisplayCommon.h"
#include "Timer.h"
#include <crazygaze/micromuc/Profiler.h>

extern uint32_t gTickCount;

namespace cz
{

extern Timer gTimer;
using namespace gfx;

namespace
{
	#define DEFINE_SENSOR_CALIBRATION_LABEL_LINE(COL, LINE, SUBLINE, FLAGS) \
	const LabelData sensorCalibrationSettings##COL##LINE##SUBLINE PROGMEM = \
	{ \
		{LayoutHelper::getMenuButtonPos(COL, LINE).x, LayoutHelper::getMenuButtonPos(COL,LINE).y + (SUBLINE * (AW_GRAPH_HEIGHT/3)), 32, AW_GRAPH_HEIGHT/3}, \
		HAlign::Center, VAlign::Center, \
		TINY_FONT, \
		AW_GRAPH_VALUES_TEXT_COLOUR, AW_GRAPH_VALUES_BKG_COLOUR, \
		WidgetFlag::EraseBkg | FLAGS \
	};

	DEFINE_SENSOR_CALIBRATION_LABEL_LINE(1, 1, 0, WidgetFlag::None)
	DEFINE_SENSOR_CALIBRATION_LABEL_LINE(1, 1, 1, WidgetFlag::NumAsPercentage)
	DEFINE_SENSOR_CALIBRATION_LABEL_LINE(1, 1, 2, WidgetFlag::None)
	DEFINE_SENSOR_CALIBRATION_LABEL_LINE(4, 1, 1, WidgetFlag::NumAsPercentage) // For the data we show where we can use the minus/plus to change the threshold

	// Defines the label data for menus, where a single cell can take 2 lines
	#define DEFINE_LABEL_LINE2(COL, name, CELL_LINE, FLAGS) \
	const LabelData labelData_##name##_##CELL_LINE PROGMEM = \
	{ \
		{LayoutHelper::getMenuButtonPos(1+COL, 1).x, LayoutHelper::getMenuButtonPos(1+COL,1).y + (CELL_LINE * (AW_GRAPH_HEIGHT/2)), 32, AW_GRAPH_HEIGHT/2}, \
		HAlign::Center, VAlign::Center, \
		TINY_FONT, \
		AW_GRAPH_VALUES_TEXT_COLOUR, AW_GRAPH_VALUES_BKG_COLOUR, \
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
			{ sensorCalibrationSettings110},
			{ sensorCalibrationSettings111},
			{ sensorCalibrationSettings112},
		}
	, m_calibrationManualThresholdLabel{sensorCalibrationSettings411}
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
	auto initButton = [this](ButtonId id, auto&&... params) -> gfx::ImageButton&
	{
		m_buttons[(int)id].init((int)id, std::forward<decltype(params)>(params)...);
		return m_buttons[(int)id];
	};

	// First line
	initButton(ButtonId::CloseAndSave, LayoutHelper::getMenuButtonPos(0,0), AW_SCREEN_BKG_COLOUR, img_Save);
	initButton(ButtonId::Calibrate, LayoutHelper::getMenuButtonPos(CALIBRATE_BTN_COL,0), AW_SCREEN_BKG_COLOUR, img_Ruler);
	initButton(ButtonId::SensorInterval, LayoutHelper::getMenuButtonPos(SENSORINTERVAL_BTN_COL,0), AW_SCREEN_BKG_COLOUR, img_SetSensorInterval);
	initButton(ButtonId::ShotDuration, LayoutHelper::getMenuButtonPos(SHOTDURATION_BTN_COL,0), AW_SCREEN_BKG_COLOUR, img_SetWateringDuration);
	// Leaving one empty grid space intentionally, so the CloseAndIgnore button is spaced away from the others
	initButton(ButtonId::CloseAndIgnore, LayoutHelper::getMenuButtonPos(5,0), AW_SCREEN_BKG_COLOUR, img_Close);

	// Second line
	initButton(ButtonId::ResetRange, LayoutHelper::getMenuButtonPos(CALIBRATE_BTN_COL-1,1), AW_SCREEN_BKG_COLOUR, img_Eraser).setClearWhenHidden(false);
	initButton(ButtonId::SetThreshold, LayoutHelper::getMenuButtonPos(CALIBRATE_BTN_COL+1,1), AW_SCREEN_BKG_COLOUR, img_SetThreshold).setClearWhenHidden(false);
	initButton(ButtonId::Minus, LayoutHelper::getMenuButtonPos(0,1), AW_SCREEN_BKG_COLOUR, img_Remove).setClearWhenHidden(false);
	initButton(ButtonId::Plus, LayoutHelper::getMenuButtonPos(0,1), AW_SCREEN_BKG_COLOUR, img_Add).setClearWhenHidden(false);

}

void SettingsMenu::tick(float deltaSeconds)
{
	PROFILE_SCOPE(F("SettingsMenu"));

	draw();
}

void SettingsMenu::setState(State state)
{
	//CZ_LOG(logDefault, Log, F("%s(%d):TickCount=%u"), __FUNCTION__, (int)state, gTickCount);

	m_clearMenuSecondLine = true;
	m_state = state;

	m_buttons[(int)ButtonId::CloseAndSave].setVisible(true);
	m_buttons[(int)ButtonId::CloseAndSave].setEnabled(m_dummyCfg.isDirty());

	//
	//	These are always visible regardless of the sub-menu we're in, although they can be in a disabled state
	m_buttons[(int)ButtonId::Calibrate].setVisible(true);
	m_buttons[(int)ButtonId::Calibrate].setEnabled(state==State::Main || state==State::CalibratingSensor);
	m_buttons[(int)ButtonId::SensorInterval].setVisible(true);
	m_buttons[(int)ButtonId::SensorInterval].setEnabled(state==State::Main || state==State::SettingSensorInterval);
	m_buttons[(int)ButtonId::ShotDuration].setVisible(true);
	m_buttons[(int)ButtonId::ShotDuration].setEnabled(state==State::Main || state==State::SettingShotDuration);

	m_buttons[(int)ButtonId::CloseAndIgnore].setVisible(true);
	m_buttons[(int)ButtonId::CloseAndIgnore].setEnabled(true);

	bool showMinusPlus = (m_state==State::CalibratingSensor || m_state==State::SettingSensorInterval || m_state==State::SettingShotDuration);
	setButton(ButtonId::ResetRange, state==State::CalibratingSensor, state==State::CalibratingSensor);
	setButton(ButtonId::SetThreshold, state==State::CalibratingSensor, state==State::CalibratingSensor);
	setButton(ButtonId::Minus, showMinusPlus, showMinusPlus);
	setButton(ButtonId::Plus, showMinusPlus, showMinusPlus);

	if (m_state==State::CalibratingSensor)
	{
		m_buttons[(int)ButtonId::Minus].move(LayoutHelper::getMenuButtonPos(CALIBRATE_BTN_COL+2, 1), true);
		m_buttons[(int)ButtonId::Plus ].move(LayoutHelper::getMenuButtonPos(CALIBRATE_BTN_COL+4, 1), true);
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

	if (state==State::Main || state==State::CalibratingSensor)
	{
		setSensorLabels();
	}
	else
	{
		for(auto&& l : m_calibrationLabels)
		{
			l.clearValue();
		}
		m_calibrationManualThresholdLabel.clearValue();
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
		m_samplingIntervalLabels[0].clearValue();
		m_samplingIntervalLabels[1].clearValue();
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
		m_shotDurationLabels[0].clearValue();
		m_shotDurationLabels[1].clearValue();
	}

	//
	// Set all labels as dirty to force a draw or erase to reflect the state we setup in this function
	//
	for(auto&& l : m_calibrationLabels)
	{
		l.setDirty(true);
	}
	m_calibrationManualThresholdLabel.setDirty(true);

	for(auto&& l : m_samplingIntervalLabels)
	{
		l.setDirty(true);
	}
	for(auto&& l: m_shotDurationLabels)
	{
		l.setDirty(true);
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

	// gets a scaling factor for the Minus/Plus buttons, based on how fast we are clicking them
	auto getScalingFactor = [this]() -> int
	{
		uint64_t deltaMs = (gTimer.getTotalMicros() - m_lastMinusPlusPressTimeMicros) / 1000;
		m_lastMinusPlusPressTimeMicros = gTimer.getTotalMicros();

		if (deltaMs > 300)
		{
			return 1;
		}
		else
		{
			return 10;
		}
	};

	if (checkButtonPressed(ButtonId::CloseAndSave))
	{
		if (m_dummyCfg.isDirty())
		{
			gCtx.data.getSelectedGroup()->setConfig(m_dummyCfg);
			// This needs to be AFTER setConfig, so listeners can read the new config data if they need it
			m_dummyCfg.endCalibration();

			gCtx.data.saveGroupConfig(gCtx.data.getSelectedGroupIndex());
		}
		return true;
	}
	else if (checkButtonPressed(ButtonId::CloseAndIgnore))
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
	else if (checkButtonPressed(ButtonId::ResetRange))
	{
		// NOTE : The only way this button should respond is if we are in the CalibratingSensor state already
		assert(m_state==State::CalibratingSensor);
		// Once we click the button, we mark it as disabled just for visible feedback. If the user wants to hit reaset again, it needs to get out of the menu and join again.
		setButton(ButtonId::ResetRange, false, true);
		m_dummyCfg.startCalibration();
		return true;
	}
	else if (checkButtonPressed(ButtonId::SetThreshold))
	{
		// NOTE : The only way this button should respond is if we are in the CalibratingSensor state already
		assert(m_state==State::CalibratingSensor);
		m_dummyCfg.setThresholdValue(m_dummyCfg.getCurrentValue());
		setState(State::Main);
		return true;
	}
	else if (checkButtonPressed(ButtonId::Minus))
	{
		if (m_state==State::CalibratingSensor)
		{
			// NOTE: Higher values (higher voltage returned by the sensor), means drier
			changeThresholdValue(+1 * getScalingFactor());
		}
		else if (m_state==State::SettingSensorInterval)
		{
			changeSamplingInterval(-1 * getScalingFactor());
		}
		else if (m_state==State::SettingShotDuration)
		{
			changeShotDuration(-1 * getScalingFactor());
		}
		else
		{
			CZ_UNEXPECTED();
		}
		return true;
	}
	else if (checkButtonPressed(ButtonId::Plus))
	{
		if (m_state==State::CalibratingSensor)
		{
			// NOTE: Lower values (low voltage returned by the sensor), means wetter
			changeThresholdValue(-1 * getScalingFactor());
		}
		else if (m_state==State::SettingSensorInterval)
		{
			changeSamplingInterval(+1 * getScalingFactor());
		}
		else if (m_state==State::SettingShotDuration)
		{
			changeShotDuration(+1 * getScalingFactor());
		}
		else
		{
			CZ_UNEXPECTED();
		}
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

	if (m_clearMenuSecondLine)
	{
		m_clearMenuSecondLine = false;
		fillRect(LayoutHelper::getMenuLineArea(1), AW_SCREEN_BKG_COLOUR);
	}

	if (data)
	{
		if (m_state==State::Main || m_state==State::CalibratingSensor)
		{
			for(auto&& l : m_calibrationLabels)
				l.draw();

			if (m_state==State::CalibratingSensor)
			{
				// This label shows between the minus and plus, and ONLY when in the calibrating submenu
				m_calibrationManualThresholdLabel.draw();
			}
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

	//
	// Buttons need to be drawn AFTER the labels, so the labels don't overwrite the Minus/Plus buttons
	for(gfx::ImageButton& btn : m_buttons)
	{
		btn.draw(m_forceDraw);
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
	//CZ_LOG(logDefault, Log, F("%s:TickCount=%u"), __FUNCTION__, gTickCount);

	Menu::show();

	GroupData* data = gCtx.data.getSelectedGroup();
	m_dummyCfg = data->copyConfig();
	data->setInConfigMenu(true);
	setState(State::Main);
}

void SettingsMenu::setSensorLabels()
{
	//CZ_LOG(logDefault, Log, F("%s:TickCount=%u"), __FUNCTION__, gTickCount);
	m_calibrationLabels[0].setValue(m_dummyCfg.getWaterValue());
	if (m_state == State::CalibratingSensor)
	{
		// If in the calibration sub-menu we want this label to reflect the current sensor reading, so the user knows what the threshold
		// will be if he hits the "set threshold button"
		m_calibrationLabels[1].setValue(m_dummyCfg.getCurrentValueAsPercentage());
		m_calibrationManualThresholdLabel.setValue(m_dummyCfg.getThresholdValueAsPercentage());
	}
	else
	{
		// If NOT in the calibration sub-menu, we want this label to show what the current threshold is (in the dummy config)
		m_calibrationLabels[1].setValue(m_dummyCfg.getThresholdValueAsPercentage());
	}

	m_calibrationLabels[2].setValue(m_dummyCfg.getAirValue());
}

void SettingsMenu::changeThresholdValue(int direction)
{
	int currentValue = (int)m_dummyCfg.getThresholdValue();
	int newValue = cz::clamp<int>(currentValue + (direction * m_dummyCfg.getThresholdValueOnePercent()), 0, (1<<AW_ADC_NUM_BITS)-1);
	m_dummyCfg.setThresholdValue(newValue);
	CZ_LOG(logDefault, Log, F("%s(%d): %d -> %d"), __FUNCTION__, direction, currentValue, newValue);
}

void SettingsMenu::changeSamplingInterval(int direction)
{
	int currentSamplingInterval = m_dummyCfg.getSamplingInterval();
	// Remove any inconsistencies in the settings
	// * The config works internally as seconds, but the UI works in minutes
	// * Intentionally converting to minutes and back to seconds
	currentSamplingInterval = (currentSamplingInterval/60) * 60;

	int newSamplingInterval = currentSamplingInterval + direction*60;
	// >=0 because we want to allow 0 to be passed down to the config.
	// This is because the config expected a value in seconds, but the UI uses minutes for the sampling interval, and if we clamped here in minutes, 
	// the minimum sampling interval would be 1 minute which is too high for testing.
	// So, we allow passing 0 (minutes) down to the config, and it gets clamped there to 1 second.
	if (newSamplingInterval >= 0)
	{
		m_dummyCfg.setSamplingInterval(newSamplingInterval);
		m_samplingIntervalLabels[0].setText(*IntToString(static_cast<int>(m_dummyCfg.getSamplingIntervalInMinutes())));
	}
}

void SettingsMenu::changeShotDuration(int direction)
{
	int newShotDuration = (int)m_dummyCfg.getShotDuration() + direction;
	if (newShotDuration > 0)
	{
		m_dummyCfg.setShotDuration(m_dummyCfg.getShotDuration() + direction);
		m_shotDurationLabels[0].setText(*IntToString(static_cast<int>(m_dummyCfg.getShotDuration())));
	}
}

void SettingsMenu::hide()
{
	// Set all buttons and labels as hidden, so they will be marked as dirty when we enter this menu a second time
	// Even though Menu::hide clears the entire menu area, this is still needed, so the UI elements are marked dirty when we
	// enter the menu again
	setButtonRange(ButtonId::First, ButtonId::Max, false, false);
	for(auto&& l : m_calibrationLabels)
	{
		l.clearValue();
	}
	m_calibrationManualThresholdLabel.clearValue();

	for(auto&& l : m_samplingIntervalLabels)
	{
		l.clearValue();
	}
	for(auto&& l : m_shotDurationLabels)
	{
		l.clearValue();
	}

	GroupData* data = gCtx.data.getSelectedGroup();
	data->setInConfigMenu(false);

	Menu::hide();
}

bool SettingsMenu::checkClose()
{
	if (m_pressedId == ButtonId::CloseAndSave)
	{
		m_pressedId = ButtonId::Max;
		return true;
	}
	else if (m_pressedId == ButtonId::CloseAndIgnore)
	{
		m_pressedId = ButtonId::Max;
		return true;
	}

	return false;
}

} // namespace cz
