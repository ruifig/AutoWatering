#pragma once

#include "Context.h"
#include <Adafruit_RGBLCDShield.h>

namespace cz
{

class DisplayLCD
{
  public:
	DisplayLCD(Context& ctx);

	// Disable copying
	DisplayLCD(const DisplayLCD&) = delete;
	DisplayLCD& operator=(const DisplayLCD&) = delete;

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
	Adafruit_RGBLCDShield m_lcd;
	float m_timeInState = 0;
	float m_lastButtonPress = 0;
	State m_state = State::Initializing;
	bool m_backlightEnabled = false;

	void setBacklight(bool state);
	void changeToState(State newState);
	void onLeaveState();
	void onEnterState();
};

}  // namespace cz