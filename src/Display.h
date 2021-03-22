#pragma once

#include "ProgramData.h"
#include <Adafruit_RGBLCDShield.h>

namespace cz
{

class Display
{
  public:
	Display() {}

	// Disable copying
	Display(const Display&) = delete;
	Display& operator=(const Display&) = delete;

	void setup(Adafruit_RGBLCDShield& lcd, ProgramData& data);
	float tick(float deltaSeconds);

  private:

    enum class State : uint8_t
    {
        Initializing,
        Intro,
        Overview
    };

	Adafruit_RGBLCDShield* m_lcd = nullptr;
	ProgramData* m_data = nullptr;
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