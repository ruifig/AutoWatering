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
	uint8_t m_soilMoistureSensorUpdates[NUM_MOISTURESENSORS];
	bool m_forceDrawOnNextTick = false;

	SensorMainMenu m_sensorMainMenu;

	void changeToState(State newState);
	void onLeaveState();
	void onEnterState();

	void drawOverview();
	void drawHistoryBoxes();

	/**
	 * Plots a dot graph starting at x,y, with height h.
	 * One graph point per value (along the x axis), and the value range (vertical) is mapped so that:
	 * - A value of 0 is drawn (y+h)
	 * - A value of 100 is drawn at (y)
	 *
	 * @param x,y Top left corner
	 * @param h Height to map the values (a maximum value takes makes the graph h in height)
	 * @param data/count data to plot
	 * @param valThreshold If a value <= this, then it uses colour GRAPH_MOISTURE_LOW_COLOUR, otherwise GRAPH_MOISTURE_OK_COLOUR
	 * @param oldData/oldCount if specified, the data being plotted is compared against this, so it draws as few pixels as possible
	 *    If this is not specified, then the function erases a full Y range per point before drawing the point.
	 **/
	void plotHistory(int16_t x, int16_t y, int16_t h, const TFixedCapacityQueue<GraphPoint>& data, int previousDrawOffset, uint8_t valThreshold /*, const GraphPoint* oldData = nullptr, int oldCount=0*/);


};
	
	
} // namespace cz




