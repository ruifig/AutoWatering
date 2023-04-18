#pragma once

#include "Config/Config.h"
#include "Component.h"
#include "GroupGraph.h"
#include "gfx/MyDisplay1.h"
#include "crazygaze/TouchController/XPT2046.h"

#include "SettingsMenu.h"
#include "MainMenu.h"
#include "ShotConfirmationMenu.h"

namespace cz
{

class GraphicalUI : public Component
{
  public:
	GraphicalUI();
	
	// Disable copying
	GraphicalUI(const GraphicalUI&) = delete;
	GraphicalUI& operator=(const GraphicalUI&) = delete;

	void scrollSlots(int inc);

  private:

	//
	// Component interface
	//
	virtual const char* getName() const override { return "GraphicalUI"; }
	virtual bool initImpl() override;
	virtual float tick(float deltaSeconds) override;
	virtual void onEvent(const Event& evt) override;
	virtual bool processCommand(const Command& cmd) override;

	//
	// DisplayState
	//
	class DisplayState
	{
	public:
		DisplayState(GraphicalUI& outer) : m_outer(outer) {}
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
		GraphicalUI& m_outer;
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
		OverviewState(GraphicalUI& outer);
	#if CZ_LOG_ENABLED
		virtual const char* getName() const { return "Overview"; }
	#endif
		virtual void init() override;
		virtual void tick(float deltaSeconds) override;
		virtual void onEnter() override;
		virtual void onLeave() override;
		virtual void onEvent(const Event& evt) override;

		void scrollSlots(int inc);
	protected:

		void draw();
		void drawGroupNumbers();

		struct Group
		{
			GroupGraph graph;
			bool sensorUpdates = false;
		};

		Group m_groups[VISIBLE_NUM_PAIRS];
		Group* tryGetGroupByPairIndex(int8_t pairIndex);
		Group& getGroupByPairIndex(int8_t pairIndex);

		MainMenu m_sensorMainMenu;
		SettingsMenu m_settingsMenu;
		ShotConfirmationMenu m_shotConfirmationMenu;
		Menu* m_currentMenu = nullptr;
		// Instead of keepin track of time passed to update the running time label every second
		// we just look for a change in the seconds part.
		uint8_t m_previousRunningTimeSeconds=255;
		// What pair is in the first screen slot
		int8_t m_topSlotPairIndex = 0;
		bool m_forceRedraw : 1;
	};

	struct States
	{
		States(GraphicalUI& outer)
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
		// A touch&release happened, and we should process it
		bool pressed = false;
		Pos pos; // Position to process as clicked

		// This is set to true when we want to sleep and we set turn off the screen backlight.
		// When a touch is process, if this is true, that one touch will be ignored, and we wake up/turn on the backlight
		bool sleeping = false;
		float secondsSinceLastTouch = 0;

		// used to slowly dim the brightness to 0 when putting to sleep.
		// This is better than simply turning off the backlight, so the user knows it's a fault. 
		float currentBrightness = 0;
	} m_touch;

	void updateTouch(float deltaSeconds);
};

extern GraphicalUI gGraphicalUI;
	
} // namespace cz

