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

#if LOG_ENABLED
	static const char* ms_stateNames[3];
#endif

	Context& m_ctx;
	MCUFRIEND_kbv m_tft;
	TouchScreen m_ts;
	State m_state = State::Initializing;
	float m_timeInState = 0;
	float m_timeSinceLastTouch = 0;
	bool m_screenOff = false;

	void changeToState(State newState);
	void onLeaveState();
	void onEnterState();

};
	
	
} // namespace cz




