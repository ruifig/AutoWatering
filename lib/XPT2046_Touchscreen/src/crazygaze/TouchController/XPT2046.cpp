#include "XPT2046.h"

#define Z_THRESHOLD     300

namespace cz
{

void XPT2046::begin(SPIClass& wspi)
{
	m_ts.begin(wspi);
}

void XPT2046::updateState()
{
	uint16_t x,y;
	uint8_t z;
	bool touching;
	m_ts.readData(&x, &y, &z, &touching);
	RawTouchPoint raw(x,y,z);

	if (touching)
	{
		m_lastRawPoint = raw;
		m_lastPoint = convertRawToScreen(raw);
	}

	switch(m_state)
	{
		case TouchState::Idle:
		{
			if (touching)
			{
				m_state = TouchState::Touching;
			}
		}
		break;

		case TouchState::Touching:
		{
			if (!touching)
			{
				m_state = TouchState::Released;
			}
		}
		break;

		case TouchState::Released:
		{
			if (touching)
			{
				m_state = TouchState::Touching;
			}
			else
			{
				m_state = TouchState::Idle;
			}
		}
		break;
	}

}

} // namespace cz
