#pragma once

#include <utility/Adafruit_MCP23017.h>

#include "Config.h"

namespace cz
{
class MCP23017Wrapper
{
  public:
	/**
	 * @param addr 0..7
	 * 0x20 is added to this internally
	 */
	void begin(uint8_t addr)
	{
		// The library accepts a 0..7 (it adds 0x20 internally), so we need to convert to 0..7
		m_inner.begin(addr - 0x20);
	}
	void begin(void) { m_inner.begin(); }

	void pinMode(IOExpanderPin p, uint8_t d) { m_inner.pinMode(p.raw, d); }
	void digitalWrite(IOExpanderPin p, uint8_t d) { m_inner.digitalWrite(p.raw, d); }
	void pullUp(IOExpanderPin p, uint8_t d) { m_inner.pullUp(p.raw, d); }
	uint8_t digitalRead(IOExpanderPin p) { return m_inner.digitalRead(p.raw); }

	void writeGPIOAB(uint16_t d) { m_inner.writeGPIOAB(d); }
	uint16_t readGPIOAB() { return m_inner.readGPIOAB(); }

  private:
	Adafruit_MCP23017 m_inner;
};

}  // namespace cz
