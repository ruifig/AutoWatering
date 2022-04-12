#pragma once
#include "Component.h"
#include "Config.h"
#include "gfx/Label.h"
#include "gfx/Button.h"
#include "GroupGraph.h"
#include "gfx/MyDisplay1.h"
#include "gfx/MyXPT2046.h"

namespace cz
{

class Menu
{
  public:
	Menu() {}
	virtual ~Menu() {}

	virtual void init() = 0;
	virtual void tick(float deltaSeconds) = 0;
	virtual void onEvent(const Event& evt) = 0;
	virtual bool processTouch(const Pos& pos) = 0;
		
	void setForceDraw()
	{
		m_forceDraw = true;
	}
	
  protected:
	virtual void draw() = 0;
	bool m_forceDraw = false;
};

class SensorMainMenu : public Menu
{
  public:
	SensorMainMenu() {}
	virtual ~SensorMainMenu() {}

	virtual void init() override;
	virtual void tick(float deltaSeconds) override;
	virtual void onEvent(const Event& evt) override;
	virtual bool processTouch(const Pos& pos) override;

	void show();
	void hide();
	bool checkShowSettings();

  protected:
	virtual void draw() override;
	void updateButtons();

	enum class ButtonId : uint8_t
	{
		StartGroup,
		StopGroup,
		Shot,
		Settings,
		Max
	};
	gfx::ImageButton m_buttons[(int)ButtonId::Max];
	bool m_showSettings : 1;
};

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

class DisplayTFT : public Component
{
  public:
	DisplayTFT();
	
	// Disable copying
	DisplayTFT(const DisplayTFT&) = delete;
	DisplayTFT& operator=(const DisplayTFT&) = delete;

	void begin();
	virtual float tick(float deltaSeconds) override;
	virtual void onEvent(const Event& evt) override;

  private:

	//
	// DisplayState
	//
	class DisplayState
	{
	public:
		DisplayState(DisplayTFT& outer) : m_outer(outer) {}
		virtual ~DisplayState() {}
	#if CZ_LOG_ENABLED
		virtual const char* getName() const = 0;
	#endif
		virtual void init() {}
		virtual void tick(float deltaSeconds) = 0;
		virtual void onEnter() = 0;
		virtual void onLeave() = 0;
		virtual void onEvent(const Event& evt) {}
	protected:
		DisplayTFT& m_outer;
	};

	//
	// InitializeState
	//
	class InitializeState : public DisplayState
	{
	public:
		using DisplayState::DisplayState;
	#if CZ_LOG_ENABLED
		virtual const char* getName() const { return "Initialize"; }
	#endif
		virtual void tick(float deltaSeconds) override;
		virtual void onEnter() override;
		virtual void onLeave() override;
	protected:
	};

	//
	// IntroState
	//
	class IntroState : public DisplayState
	{
	public:
		using DisplayState::DisplayState;
	#if CZ_LOG_ENABLED
		virtual const char* getName() const { return "Intro"; }
	#endif
		virtual void tick(float deltaSeconds) override;
		virtual void onEnter() override;
		virtual void onLeave() override;
	protected:
	};

	//
	// BootMenu
	//
	class BootMenuState : public DisplayState
	{
	  public:
		using DisplayState::DisplayState;
	#if CZ_LOG_ENABLED
		virtual const char* getName() const { return "BootMenu"; }
	#endif
		virtual void init() override;
		virtual void tick(float deltaSeconds) override;
		virtual void onEnter() override;
		virtual void onLeave() override;
	  protected:

		float m_defaultConfigCountdown = BOOTMENU_COUNTDOWN;
		bool m_waitingResetConfirmation = false;
	};


	//
	// OverviewState
	//
	class OverviewState : public DisplayState
	{
	public:
		OverviewState(DisplayTFT& outer);
	#if CZ_LOG_ENABLED
		virtual const char* getName() const { return "Overview"; }
	#endif
		virtual void init() override;
		virtual void tick(float deltaSeconds) override;
		virtual void onEnter() override;
		virtual void onLeave() override;
		virtual void onEvent(const Event& evt) override;

	protected:

		void draw();

		GroupGraph m_groupGraphs[NUM_PAIRS];
		uint8_t m_sensorUpdates[NUM_PAIRS];
		SensorMainMenu m_sensorMainMenu;
		SettingsMenu m_settingsMenu;
		// Instead of keepin track of time passed to update the running time label every second
		// we just look for a change in the seconds part.
		uint8_t m_previousRunningTimeSeconds=255;
		bool m_forceRedraw : 1;
		bool m_inSettingsMenu : 1;
	};

	struct States
	{
		States(DisplayTFT& outer)
			: initialize(outer)
			, intro(outer)
			, bootMenu(outer)
			, overview(outer)
		{
		}
		InitializeState initialize;
		IntroState intro;
		BootMenuState bootMenu;
		OverviewState overview;
	} m_states;

	DisplayState* m_state = nullptr;
	float m_timeInState = 0;

	void changeToState(DisplayState& newState);

	struct
	{
		bool pressed;
		Pos pos;
		// A "press" condition requires us to touch and untouch the screen, so we need to hold the previous value
		// to compare against in the next tick
		TouchPoint tmp;
	} m_touch;

	void updateTouch();
};
	
	
} // namespace cz




