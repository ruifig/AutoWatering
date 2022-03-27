#include "MyXPT2046.h"

namespace cz
{

void MyXPT2046::begin()
{
	m_ts.begin();
}

RawTouchPoint MyXPT2046::getRawPoint()
{
	uint16_t x,y;
	uint8_t z;
	m_ts.readData(&x, &y, &z);
	return RawTouchPoint(x,y,z);
}

} // namespace cz
