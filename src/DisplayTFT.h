#pragma once
#include "Context.h"
#include "Component.h"
#include <Adafruit_GFX.h>
#include <MCUFRIEND_kbv.h>
#include <TouchScreen.h>
#include "gfx/GFXUtils.h"

namespace cz
{

class Menu
{
  public:
	Menu() {}
	virtual ~Menu() {}

	virtual void init() = 0;
	virtual void tick(float deltaSeconds) = 0;
	virtual void draw(bool forceDraw) = 0;
		
  protected:
};

class SensorMainMenu : public Menu
{
  public:
	SensorMainMenu() {}
	virtual ~SensorMainMenu() {}

	virtual void init() override;
	virtual void tick(float deltaSeconds) override;
	virtual void draw(bool forceDraw = false) override;

  protected:
	enum
	{
		Start,
		Stop,
		Shot,
		Settings,
		Max
	};
	ImageButton m_buttons[Max];
};

class DisplayTFT : public Component
{
  public:
	DisplayTFT(Context& ctx);
	
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
		bool m_forceRedraw = false;
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
		using DisplayState::DisplayState;
	#if CZ_LOG_ENABLED
		virtual const char* getName() const { return "Overview"; }
	#endif
		virtual void init() override;
		virtual void tick(float deltaSeconds) override;
		virtual void onEnter() override;
		virtual void onLeave() override;
		virtual void onEvent(const Event& evt) override;

	protected:

		void drawOverview();
		void drawHistoryBoxes();
		void plotHistory(int groupIndex);

		uint8_t m_sensorUpdates[NUM_MOISTURESENSORS];
		SensorMainMenu m_sensorMainMenu;
	};

	Context& m_ctx;
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
};
	
	
} // namespace cz




