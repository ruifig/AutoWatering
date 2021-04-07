#include "Display.h"
#include "Utils.h"

namespace cz
{

#if LOG_ENABLED
const char* Display::ms_stateNames[3] =
{
	"Initializing",
	"Intro",
	"Overview"
};
#endif


Display::Display(Context& ctx)
    : m_ctx(ctx)
{
}

void Display::begin()
{
    onEnterState();
}

float Display::tick(float deltaSeconds)
{
    m_timeInState += deltaSeconds;
    m_lastButtonPress += deltaSeconds;

    uint8_t buttons = m_lcd.readButtons();
    if (buttons)
    {
        m_lastButtonPress = 0;
        setBacklight(true);
    }

	//CZ_LOG_LN("Display::%s: state=%d, timeInState = %d", __FUNCTION__, (int)m_state, (int)m_timeInState);

	switch(m_state)
    {

        case State::Initializing:
            changeToState(State::Intro);
            break;

        case State::Intro:
            if (m_timeInState >= INTRO_DURATION)
            {
                changeToState(State::Overview);
            }
            break;

        case State::Overview:
            // TODO : Fill me
            break;

        default:
            CZ_UNEXPECTED();
    }

    if (m_lastButtonPress >= DEFAULT_LCD_BACKLIGHT_TIMEOUT)
    {
        setBacklight(false);
    }

	return 1.0f / 5.0f;
}

void Display::setBacklight(bool state)
{
	if (state != m_backlightEnabled)
    {
        m_backlightEnabled = state;
        m_lcd.setBacklight( state ? 0x1 : 0x0);
    }
}

void Display::changeToState(State newState)
{
    CZ_LOG_LN("Display::%s %dms %s->%s"
        , __FUNCTION__
		, (int)(m_timeInState * 1000.f)
        , ms_stateNames[(int)m_state]
        , ms_stateNames[(int)newState]);

    onLeaveState();
    m_lcd.clear();
    m_state = newState;
    m_timeInState = 0.0f;
    onEnterState();
}

void Display::onLeaveState()
{
	switch (m_state)
	{
	case State::Initializing:
		break;
	case State::Intro:
		break;
	case State::Overview:
		break;

	default:
		CZ_UNEXPECTED();
	}
}

void Display::onEnterState()
{
	switch (m_state)
	{
	case State::Initializing:
        m_lcd.begin(16, 2);
        m_lcd.clear();
        setBacklight(true);
		break;
	case State::Intro:
        m_lcd.print("AutoWatering by");
        m_lcd.setCursor(0, 1);
        m_lcd.print("Rui Figueira");
		break;
	case State::Overview:
		break;

	default:
		CZ_UNEXPECTED();
	}
}


}  // namespace cz
