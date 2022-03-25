#pragma once

#include <Arduino.h>
#include <Wire.h>
#include "crazygaze/micromuc/Logging.h"

/**
 * The AT24CX chips have a possible ic2 address of 0x50..0x57
 * 
 * 
 * AT24C32	32kbits	- 128 pages, 32 bytes per page
 * AT24C64	64kbits	- 256 pages, 32 bytes per page
 * AT24C128	128kbits - 256 pages, 64 bytes per page
 * AT24C256	256kbits - 512 pages, 64 bytes per page
 */
namespace cz
{

class AT24C
{
  public:
	AT24C(uint8_t address, uint16_t numPages, uint8_t bytesPerPage, TwoWire& wire);
	bool isPresent();

	void write8(uint16_t address, uint8_t data);
	uint8_t read8(uint16_t address);

	void write16(uint16_t address, uint16_t data);
	uint16_t read16(uint16_t address);

	void write32(uint16_t address, uint32_t data);
	uint32_t read32(uint16_t address);

	uint16_t write(uint16_t address, const uint8_t* src, uint16_t len);
	uint16_t read(uint16_t address, uint8_t* dest, uint16_t len);

	/**
	 * Indicates if an error occurred since the last call to hasErrorOccurred,
	 * and clears the error state.
	 */
	bool hasErrorOccurred();

	uint8_t getPageSize() const
	{
		return m_pageSize;
	}

	uint16_t getSizeBytes() const
	{
		return m_sizeBytes;
	}

	class Ptr;

	/**
	 * Represents a single byte
	 */
	class Ref
	{
	  public:

		// Access/read operators
		uint8_t operator*() const { return m_outer.read8(m_address); }
		operator uint8_t() const { return **this; }

		// Assignment/write members
		Ref &operator=( const Ref& ref ) { return *this = *ref; }
		Ref &operator=( uint8_t in )    { m_outer.write8(m_address, in); return *this; }
		Ref &operator +=( uint8_t in )  { return *this = **this + in; }
		Ref &operator -=( uint8_t in )  { return *this = **this - in; }
		Ref &operator *=( uint8_t in )  { return *this = **this * in; }
		Ref &operator /=( uint8_t in )  { return *this = **this / in; }
		Ref &operator ^=( uint8_t in )  { return *this = **this ^ in; }
		Ref &operator %=( uint8_t in )  { return *this = **this % in; }
		Ref &operator &=( uint8_t in )  { return *this = **this & in; }
		Ref &operator |=( uint8_t in )  { return *this = **this | in; }
		Ref &operator <<=( uint8_t in ) { return *this = **this << in; }
		Ref &operator >>=( uint8_t in ) { return *this = **this >> in; }

		Ref &update( uint8_t in ) { return  in != *this ? *this = in : *this; }

		/** Prefix increment/decrement **/
		Ref& operator++() { return *this += 1; }
		Ref& operator--() { return *this -= 1; }

		/** Postfix increment/decrement **/
		uint8_t operator++ (int)
		{ 
			uint8_t ret = **this;
			return ++(*this), ret;
		}

		uint8_t operator-- (int)
		{ 
			uint8_t ret = **this;
			return --(*this), ret;
		}

	  protected:
	  	friend class AT24C::Ptr;

		Ref(AT24C& outer, uint16_t address)
			: m_outer(outer)
			, m_address(address) {}

	  private:
		AT24C& m_outer;
		uint16_t m_address;
	};

	/**
	 * Bidirectional pointer
	 * 
	 */
	class Ptr
	{
	  public:

		operator uint16_t() const { return m_address; }
		Ptr& operator=(uint16_t address) { return m_address = address, *this; }
	
		//Iterator functionality.
		bool operator!=( const Ptr& ptr ) { return m_address!=ptr.m_address || &m_outer!=&ptr.m_outer; }
		Ref operator*()				   { return Ref(m_outer, m_address); }
	
		/** Prefix & Postfix increment/decrement **/
		Ptr& operator++()	 { return ++m_address, *this; }
		Ptr& operator--()	 { return --m_address, *this; }
		Ptr operator++ (int) { return Ptr(m_outer, m_address++); }
		Ptr operator-- (int) { return Ptr(m_outer, m_address--); }

	  protected:
	  	friend class AT24C;
		Ptr(AT24C& outer, uint16_t address)
			: m_outer(outer)
			, m_address(address)
		{}

	  private:
		AT24C& m_outer;
		uint16_t m_address;
	};

	/**
	 * Intentionally not naming this as "begin()" to avoid confusion with arduino's "begin" calls
	 */
	Ptr at(uint16_t address)
	{
		CZ_ASSERT(address<m_sizeBytes);
		return Ptr(*this, address);
	}
	Ptr end()
	{
		return Ptr(*this, m_sizeBytes);
	}

  protected:
	void writeAddress(uint16_t address);
	void waitForReady();
	uint8_t calcBulkSize(uint16_t address, uint16_t len);

	// Utility functions used to compose the other ones
	void writeByte(uint16_t address, uint8_t data);
	uint8_t readByte(uint16_t address);

  private:
	static constexpr int kBaseIC2Addr = 0x50;

	uint8_t m_id;
	uint8_t m_pageSize;
	uint16_t m_sizeBytes;
	bool m_error;
	TwoWire& m_wire;
};

class AT24C32 : public AT24C
{
  public:
	AT24C32(uint8_t address, TwoWire& wire = Wire);
};

class AT24C256 : public AT24C
{
  public:
	AT24C256(uint8_t address, TwoWire& wire = Wire);
};

bool runTests(AT24C& mem);

} // namespace cz

CZ_DECLARE_LOG_CATEGORY(logAT24C, Fatal, Fatal)
