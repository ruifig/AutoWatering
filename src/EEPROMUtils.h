#pragma once

#include <EEPROM.h>
#include "AT24C.h"

namespace cz
{

class ConfigStoragePtr;

class ConfigStorage
{
  public:

	/**
	 * Called before loading/saving a config
	*/
	virtual void start() = 0;

	/**
	 * Called once loading/saving a config has finished
	*/
	virtual void end() = 0;

	/*
	* Reads a byte
	*/
	virtual uint8_t read(uint16_t address) = 0;

	/*
	* Writes a byte
	*/
	virtual void write(uint16_t address, uint8_t data) = 0;

	/**
	 * Write a byte if it's value changed
	 * This helps with flash wearing if the actual implementation allows it.
	 * E.g: Arduino-Pico's EEPROM emulation save the entire thing (I think) on commit, but the AT24C implementation
	 * only writes if indeed the value changed.
	*/
	virtual void update(uint16_t address, uint8_t data)
	{
		// Only write if the value changed, to minimize flash wearing
		if (read(address) != data)
		{
			write(address, data);
		}
	}

	ConfigStoragePtr ptrAt(uint16_t address = 0);
};

class ConfigStoragePtr
{
	public:

	explicit ConfigStoragePtr(ConfigStorage& outer, uint16_t address = 0)
		: m_outer(outer)
		, m_address(address)
	{
	}

	/**
	 * Reads a byte and sets the pointer to the next byte
	*/
	virtual uint8_t read()
	{
		uint8_t data = m_outer.read(m_address);
		m_address++;
		return data;
	}

	/**
	 * Writes a byte and sets the pointer to the next byte
	*/
	void write(uint8_t data)
	{
		m_outer.update(m_address, data);
		m_address++;
	}

	/**
	 * Sets the pointer to a new address
	 * 
	 */
	void inc(int16_t offset)
	{
		m_address += offset;
	}

	uint16_t getAddress() const
	{
		return m_address;
	}

	ConfigStorage& m_outer;
	uint16_t m_address;
};


class EEPROMWrapper : public ConfigStorage
{
  private:
	static constexpr uint16_t MAXSIZE = 4096;
  public:
	EEPROMWrapper() {}
	virtual ~EEPROMWrapper() {}

	virtual void start() override
	{
		EEPROM.begin(MAXSIZE);
	}

	virtual void end() override
	{
		EEPROM.end();
	}

	virtual uint8_t read(uint16_t address) override
	{
		CZ_ASSERT(address < MAXSIZE);
		return EEPROM.read(address);
	}

	virtual void write(uint16_t address, uint8_t data) override
	{
		CZ_ASSERT(address < MAXSIZE);
		EEPROM.write(address, data);
	}
};


/*
* Type should be "AT24C32 or AT24C256"
*/
template<typename Type>
class ATC24CWrapper : public ConfigStorage
{
  public:
	ATC24CWrapper()
		: m_at24c(0)
	{
	}

	virtual ~ATC24CWrapper()
	{
	}

	virtual void start() override { }
	virtual void end() override { }

	virtual uint8_t read(uint16_t address) override
	{
		CZ_ASSERT(address < m_at24c.getSizeBytes());
		return m_at24c.read8(address);
	}

	virtual void write(uint16_t address, uint8_t data) override
	{
		CZ_ASSERT(address < m_at24c.getSizeBytes());
		return m_at24c.write8(address, data);
	}

  private:
  Type m_at24c;
};

using ConfigStorageType = ATC24CWrapper<AT24C256>;
//using ConfigStorageType = EEPROMWrapper;

} // namespace cz

