#pragma once

#include <Arduino.h>
#include <utility/Adafruit_MCP23017.h>
#include "Config.h"

namespace cz
{

class MCP23017WrapperInterface
{
public:
	virtual void begin(uint8_t addr) = 0;
	virtual void begin() = 0;
	virtual void pinMode(IOExpanderPin p, uint8_t d) = 0;
	virtual void digitalWrite(IOExpanderPin p, uint8_t d) = 0;
	virtual void pullUp(IOExpanderPin p, uint8_t d) = 0;
	virtual uint8_t digitalRead(IOExpanderPin p) = 0;
};

class MCP23017Wrapper : public MCP23017WrapperInterface
{
  public:

	/**
	 * @param addr 0..7
	 * 0x20 is added to this internally
	 */
	virtual void begin(uint8_t addr) override
	{
		// The library accepts a 0..7 (it adds 0x20 internally), so we need to convert to 0..7
		m_inner.begin(addr - 0x20);
	}

	virtual void begin(void) override
	{
		m_inner.begin();
	}

	virtual void pinMode(IOExpanderPin p, uint8_t d) override
	{
		m_inner.pinMode(p.raw, d);
	}

	virtual void digitalWrite(IOExpanderPin p, uint8_t d) override
	{
		m_inner.digitalWrite(p.raw, d);
	}

	virtual void pullUp(IOExpanderPin p, uint8_t d) override
	{
		m_inner.pullUp(p.raw, d);
	}

	virtual uint8_t digitalRead(IOExpanderPin p) override
	{
		return m_inner.digitalRead(p.raw);
	}

	void writeGPIOAB(uint16_t d) { m_inner.writeGPIOAB(d); }
	uint16_t readGPIOAB() { return m_inner.readGPIOAB(); }

  private:
	Adafruit_MCP23017 m_inner;
};

class MockMCP23017Wrapper : public MCP23017WrapperInterface
{
public:
	/**
	 * @param addr 0..7
	 * 0x20 is added to this internally
	 */
	virtual void begin(uint8_t addr) override
	{
	}

	virtual void begin(void) override
	{
	}

	virtual void pinMode(IOExpanderPin p, uint8_t d) override
	{
	}

	virtual void digitalWrite(IOExpanderPin p, uint8_t d) override
	{
	}

	virtual void pullUp(IOExpanderPin p, uint8_t d) override
	{
	}

	virtual uint8_t digitalRead(IOExpanderPin p) override
	{
		return 0;
	}
};

}  // namespace cz
