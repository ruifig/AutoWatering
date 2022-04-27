#pragma once

#include "Menu.h"
#include "gfx/Label.h"
#include "gfx/Button.h"

namespace cz
{

class SettingsMenu : public Menu
{
  public:
	SettingsMenu();
	virtual ~SettingsMenu() {}
	virtual void init() override;
	virtual void tick(float deltaSeconds) override;
	virtual void onEvent(const Event& evt) override;
	virtual bool processTouch(const Pos& pos) override;

	virtual void show() override;
	virtual void hide() override;
	bool checkClose(bool& doSave);

  protected:
	virtual void draw() override;

	void setSensorLabels();
	void changeSamplingInterval(int direction);
	void changeShotDuration(int direction);

	enum class State : uint8_t
	{
		Main,
		CalibratingSensor,
		SettingSensorInterval,
		SettingShotDuration
	};

	State m_state = State::Main;

	enum class ButtonId : uint8_t
	{
		// First line
		First,

		CloseAndSave=First,
		Calibrate,
		SensorInterval,
		ShotDuration,
		CloseAndIgnore,

		// Second line
		SetGroupThreshold,
		Minus,
		Plus,
		
		Max
	};

	void setState(State state);

	void setButton(ButtonId idx, bool enabled, bool visible);
	// Sets a button range (inclusive
	void setButtonRange(ButtonId first, ButtonId last, bool enabled, bool visible);

	// Dummy config we act on while in the settings.
	// When getting out of the settings menu, we apply this to the real data
	GroupConfig m_dummyCfg;
	// #TODO : Implement this
	bool m_configIsDirty = false;

	gfx::ImageButton m_buttons[(int)ButtonId::Max];

	ButtonId m_pressedId = ButtonId::Max;	

	// Labels shown when in the Sensor calibration menu
	// 1: water sensor reading
	// 2: threshold setting in %
	// 3: dry sensor reading 
	// When in the main settings menu (without being inthe calibration submenu) the lines behave as:
	//    Line 1: Water sensor reading as currently saved in the settings
	//    Line 2: Threshold in %, as currently saved in the settings
	//    Line 3: Fully dry sensor reading, as currently saved in the settings
	// When inside the actual calibration sub-menu, the lines are dynamic and change accordingly to what's happening:
	//    Line 1: Current minimum sensor reading (wettest reading). This will be the new "water value" if saving these settings
	//    Line 2: Current sensor reading. This will be set as the threshold if the user presses the "Check" button
	//    Line 3: Current maximum sensor reading (driest reading). This will be the new "air value" if saving these settings
	gfx::NumLabel<true> m_calibrationLabels[3];

	// Soil sensor sampling interval labels
	// 1: number of minutes
	// 2: Text "min"
	gfx::FixedLabel<> m_samplingIntervalLabels[2];

	// Water shot duration
	// 1: number of seconds
	// 2: Text "sec"
	gfx::FixedLabel<> m_shotDurationLabels[2];

	//
	// If true, then on the next draw call, it will erase the menu second line before drawing the labels/buttons
	// This is needed because the UI framework needs some more work to support overlapping widgets.
	// For example, the second line can have labels and buttons using the same space, but hiding and show another is problematic because for example if the one hidden is processed after the visible one, it will clear up that area.
	// We solve this problem by clearing up the entire second line explicitily, and then draw processing the widgets that are visible.
	bool m_clearMenuSecondLine = false;
};

} // namespace cz

