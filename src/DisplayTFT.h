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

	enum class State : uint8_t
	{
		Initializing,
		Intro,
		Overview
	};

	static const char* const ms_stateNames[3];

	Context& m_ctx;
	TouchScreen m_ts;
	State m_state = State::Initializing;
	float m_timeInState = 0;
	float m_timeSinceLastTouch = 0;
	bool m_screenOff = false;

	uint8_t m_sensorUpdates[NUM_MOISTURESENSORS];
	bool m_forceDrawOnNextTick = false;

	SensorMainMenu m_sensorMainMenu;

	void changeToState(State newState);
	void onLeaveState();
	void onEnterState();

	void drawOverview();
	void drawHistoryBoxes();
	void plotHistory(int groupIndex);


};
	
	
} // namespace cz




