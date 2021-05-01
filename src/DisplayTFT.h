#pragma once
#include "Context.h"
#include <Adafruit_GFX.h>
#include <MCUFRIEND_kbv.h>
#include <TouchScreen.h>

namespace cz
{

class DisplayTFT
{
  public:
	DisplayTFT(Context& ctx);
	
	// Disable copying
	DisplayTFT(const DisplayTFT&) = delete;
	DisplayTFT& operator=(const DisplayTFT&) = delete;

	void begin();
	float tick(float deltaSeconds);

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

	constexpr static int16_t m_historyX = 5;
	constexpr static int16_t m_groupsStartY = 10;
	constexpr static int16_t m_spaceBetweenGroups = 20;

	struct PreviousValues
	{
		uint16_t waterValue;
		uint16_t airValue;
		uint8_t percentage;
	} m_previousValues[NUM_MOISTURESENSORS];

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
	void plotHistory(int16_t x, int16_t y, int16_t h, const TFixedCapacityQueue<GraphPoint>& data, uint8_t valThreshold /*, const GraphPoint* oldData = nullptr, int oldCount=0*/);


};
	
	
} // namespace cz




