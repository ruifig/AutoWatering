#pragma once

#include "PinTypes.h"
#include "Adafruit_MCP23017.h"

namespace cz
{

class MCP23017WrapperInterface
{
public:
	/**
	 * @param addr 0..7
	 * 0x20 is added to this internally
	 */
	virtual void begin(uint8_t addr) = 0;
	virtual void begin() = 0;
	virtual void pinMode(IOExpanderPin pin, uint8_t mode) = 0;
	virtual void digitalWrite(IOExpanderPin pin, uint8_t value) = 0;
	virtual void pullUp(IOExpanderPin pin, uint8_t value) = 0;
	virtual uint8_t digitalRead(IOExpanderPin pin) = 0;
};

class MCP23017Wrapper : public MCP23017WrapperInterface
{
  public:

	virtual void begin(uint8_t addr) override
	{
		m_inner.begin(addr);
	}

	virtual void begin(void) override
	{
		m_inner.begin();
	}

	virtual void pinMode(IOExpanderPin pin, uint8_t mode) override
	{
		m_inner.pinMode(pin.raw, mode);
	}

	virtual void digitalWrite(IOExpanderPin pin, uint8_t value) override
	{
		m_inner.digitalWrite(pin.raw, value);
	}

	virtual void pullUp(IOExpanderPin pin, uint8_t value) override
	{
		m_inner.pullUp(pin.raw, value);
	}

	virtual uint8_t digitalRead(IOExpanderPin pin) override
	{
		return m_inner.digitalRead(pin.raw);
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
