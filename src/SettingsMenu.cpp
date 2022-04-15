#include "SettingsMenu.h"
#include "Icons.h"
#include "DisplayCommon.h"

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
	initButton(ButtonId::CloseAndSave, LayoutHelper::getMenuButtonPos(0,0), SCREEN_BKG_COLOUR, img_Save);
	initButton(ButtonId::Calibrate, LayoutHelper::getMenuButtonPos(1,0), SCREEN_BKG_COLOUR, img_Ruler);
	initButton(ButtonId::SensorInterval, LayoutHelper::getMenuButtonPos(2,0), SCREEN_BKG_COLOUR, img_SetSensorInterval);
	initButton(ButtonId::ShotDuration, LayoutHelper::getMenuButtonPos(3,0), SCREEN_BKG_COLOUR, img_SetWateringDuration);
	// Leaving one empty grid space intentionally, so the CloseAndIgnore button is spaced away from the others
	initButton(ButtonId::CloseAndIgnore, LayoutHelper::getMenuButtonPos(5,0), SCREEN_BKG_COLOUR, img_Close);

	// Second line
	initButton(ButtonId::SetGroupThreshold, LayoutHelper::getMenuButtonPos(2,1), SCREEN_BKG_COLOUR, img_SetThreshold);
	initButton(ButtonId::Minus, LayoutHelper::getMenuButtonPos(1,1), SCREEN_BKG_COLOUR, img_Remove);
	initButton(ButtonId::Plus, LayoutHelper::getMenuButtonPos(3,1), SCREEN_BKG_COLOUR, img_Add);

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

} // namespace cz
