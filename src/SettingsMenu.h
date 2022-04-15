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

	void show();
	void hide();
	bool checkClose(bool& doSave);

  protected:
	virtual void draw() override;

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
		CloseAndSave,
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

	void setButton(ButtonId idx, bool enabled, bool visible);
	// Sets a button range (inclusive
	void setButtonRange(ButtonId first, ButtonId last, bool enabled, bool visible);

	// Dummy config we act on while in the settings.
	// When getting out of the settings menu, we apply this to the real data
	GroupConfig m_dummyCfg;

	gfx::ImageButton m_buttons[(int)ButtonId::Max];

	ButtonId m_pressedId = ButtonId::Max;	

	// Labels shown when in the Sensor calibration menu
	// 1: water sensor reading
	// 2: current sensor reading in %
	// 3: dry sensor reading 
	gfx::NumLabel<true> m_sensorLabels[3];

	// Soil sensor sampling interval labels
	// 1: number of minutes
	// 2: Text "min"
	gfx::FixedLabel<> m_samplingIntervalLabels[2];

	// Water shot duration
	// 1: number of seconds
	// 2: Text "sec"
	gfx::FixedLabel<> m_shotDurationLabels[2];
};

} // namespace cz

