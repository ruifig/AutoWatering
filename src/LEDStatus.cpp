#include "LEDStatus.h"
#include <Arduino.h>

namespace cz
{

bool LEDStatus::initImpl()
{
	setDefaultPattern();
	pinMode(LED_BUILTIN, OUTPUT);
	return true;
}

void LEDStatus::setPattern(PatternMode mode, bool a, bool b, bool c, bool d, bool e, bool f)
{
	m_mode = mode;
	m_pattern[0] = static_cast<uint8_t>(a);
	m_pattern[1] = static_cast<uint8_t>(b);
	m_pattern[2] = static_cast<uint8_t>(c);
	m_pattern[3] = static_cast<uint8_t>(d);
	m_pattern[4] = static_cast<uint8_t>(e);
	m_pattern[5] = static_cast<uint8_t>(f);
	m_patternIndex = 0;
}

void LEDStatus::setDefaultPattern()
{
	setPattern(PatternMode::Repeat, true, false, false, false, false, false);
}

void LEDStatus::setSuccessPattern()
{
	setPattern(PatternMode::Reset, true, false, true, false, true, false);
}

void LEDStatus::set(bool on)
{
	//CZ_LOG(logDefault, Log, "****** TURNED LED %s", on ? "ON" : "OFF");
	digitalWrite(LED_BUILTIN, on ? HIGH : LOW);
}

float LEDStatus::tick(float deltaSeconds)
{
	bool v = m_pattern[m_patternIndex];
	m_patternIndex = (m_patternIndex + 1) % 6;
	set(v);

	if (m_mode == PatternMode::Reset && m_patternIndex == 0)
	{
		setDefaultPattern();
	}

	return 0.250f;
}

void LEDStatus::onEvent(const Event& evt)
{
	switch(evt.type)
	{
		case Event::Type::WifiConnecting:
		{
			// Since connecting to Wifi can take a long while, we enable the LED before we start
			set(true);
		}
		break;
		case Event::Type::WifiStatus:
		{
			auto&& e = static_cast<const WifiStatusEvent&>(evt);
			if (e.connected)
			{
				setSuccessPattern();
			}
		}
		break;
	}
}

bool LEDStatus::processCommand(const Command& cmd)
{
	return false;
}

#if AW_LEDSTATUS_ENABLED
	LEDStatus gLEDStatus;
#endif
}
