#pragma once
#include "Component.h"
#include "Config.h"
#include "gfx/Label.h"
#include "gfx/Button.h"
#include "GroupGraph.h"
#include <Adafruit_GFX.h>
#include <MCUFRIEND_kbv.h>
#include <TouchScreen.h>

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

	void enable();
	void disable();

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
	bool m_forceDraw = false;
};

class SettingsMenu : public Menu
{
  public:
	SettingsMenu() {}
	virtual ~SettingsMenu() {}
	virtual void init() override;
	virtual void tick(float deltaSeconds) override;
	virtual void onEvent(const Event& evt) override;
	virtual bool processTouch(const Pos& pos) override;

	void show();
	void hide();

  protected:
	virtual void draw() override;

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


	gfx::ImageButton m_buttons[(int)ButtonId::Max];
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

		GroupGraph m_groupGraphs[NUM_MOISTURESENSORS];
		bool m_forceRedraw;
		uint8_t m_sensorUpdates[NUM_MOISTURESENSORS];
		SensorMainMenu m_sensorMainMenu;
		SettingsMenu m_settingsMenu;

		// 0...N-1, or 255 if no group selected
		uint8_t m_selectedGroup = 255;
	};

	TouchScreen m_ts;

	struct States
	{
		States(DisplayTFT& outer)
			: initialize(outer)
			, intro(outer)
			, overview(outer)
		{
		}
		InitializeState initialize;
		IntroState intro;
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
		TSPoint tmp;
	} m_touch;

	void updateTouch();

};
	
	
} // namespace cz




