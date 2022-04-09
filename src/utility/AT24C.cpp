#include "AT24C.h"

CZ_DEFINE_LOG_CATEGORY(logAT24C)

#if __MBED__
	// arduino::MbedI2C buffer length is 256, so we try to match it
	#define BUFFER_LENGTH 256
#else
	#ifndef BUFFER_LENGTH
		#define BUFFER_LENGTH 32
	#endif
#endif

namespace cz
{

/**
 * AT24C - Base class
 */
AT24C::AT24C(uint8_t address, uint16_t numPages, uint8_t bytesPerPage, TwoWire& wire)
	: m_id(kBaseIC2Addr | (address & 0x7))
	, m_pageSize(bytesPerPage)
	, m_sizeBytes(numPages * bytesPerPage)
	, m_error(false)
	, m_wire(wire)
{
}

bool AT24C::isPresent()
{
	m_wire.beginTransmission(m_id);
	return m_wire.endTransmission() == 0 ? true : false;
}

bool AT24C::hasErrorOccurred()
{
	bool ret = m_error;
	m_error = false;
	return ret;
}

void AT24C::writeAddress(uint16_t address)
{
	m_wire.write(address >> 8); // MSB
	m_wire.write(address & 0xFF); // LSB
}

void AT24C::waitForReady()
{
	uint16_t triesLeft = 100;

	while (!isPresent() && triesLeft != 0)
	{
		delayMicroseconds(50);
		triesLeft--;
	}

	if (triesLeft == 0)
	{
		m_error = true;
	}
}

void AT24C::write8(uint16_t address, uint8_t data)
{
	// We only write if it's a different value
	if (read8(address) == data)
		return;
	writeByte(address, data);
}

uint8_t AT24C::read8(uint16_t address)
{
	waitForReady();
	return readByte(address);
}

void AT24C::write16(uint16_t address, uint16_t data)
{
	if (read16(address) == data)
		return;
	writeByte(address, (uint8_t)data);			 // LSB
	writeByte(address + 1, (uint8_t)(data >> 8U)); // MSB
}

uint16_t AT24C::read16(uint16_t address)
{
	waitForReady();

	uint16_t ret = readByte(address);			 // LSB
	ret |= (uint16_t)readByte(address + 1) << 8U; // MSB
	return ret;
}

void AT24C::write32(uint16_t address, uint32_t data)
{
	if (read32(address) == data)
		return;

	writeByte(address++, (uint8_t) data);
	writeByte(address++, (uint8_t)(data >> 8U));
	writeByte(address++, (uint8_t)(data >> 16U));
	writeByte(address  , (uint8_t)(data >> 24U));
}

uint32_t AT24C::read32(uint16_t address)
{
	waitForReady();

	uint32_t ret = (uint32_t)readByte(address++);
	ret |= ((uint32_t)readByte(address++)) << 8U;
	ret |= ((uint32_t)readByte(address++)) << 16U;
	ret |= ((uint32_t)readByte(address)) << 24U;
	return ret;
}

uint16_t AT24C::write(uint16_t address, const uint8_t* src, uint16_t len)
{
	CZ_LOG(logDefault, Verbose, F("AT24::write(%u, %p, %u)"), (unsigned)address, src, (unsigned)len);
	CZ_ASSERT(address<m_sizeBytes);

	uint16_t todo = len;
	while(todo)
	{
		uint16_t bytes = calcBulkSize(address, todo);
		waitForReady();

		CZ_LOG(logDefault, Verbose, F("	address=%u, bytes=%u"), (unsigned)address, (unsigned)bytes);
		m_wire.beginTransmission(m_id);
		writeAddress(address);
		todo -= m_wire.write(src, bytes);
		m_wire.endTransmission();
		address += bytes;
		src += bytes;
	}

	CZ_LOG(logDefault, Verbose, F("   todo = %u"), todo);
	return len - todo;
}

uint16_t AT24C::read(uint16_t address, uint8_t* dest, uint16_t len)
{
	CZ_LOG(logDefault, Verbose, F("AT24::read(%u, %p, %u)"), (unsigned)address, dest, (unsigned)len);

	uint16_t todo = len;
	while(todo)
	{
		uint8_t bytes = calcBulkSize(address, todo);
		waitForReady();

		CZ_LOG(logDefault, Verbose, F("	address=%u, bytes=%u"), (unsigned)address, (unsigned)bytes);
		m_wire.beginTransmission(m_id);
		writeAddress(address);
		m_wire.endTransmission();

		todo -= bytes;
		address += bytes;
		m_wire.requestFrom(m_id, bytes);
		while(bytes--)
		{
			*dest = (uint8_t)m_wire.read();
			dest++;
		}
	}

	CZ_LOG(logDefault, Verbose, F("   todo = %u"), todo);
	return len - todo;
}

void AT24C::writeByte(uint16_t address, uint8_t data)
{
	waitForReady();

	m_wire.beginTransmission(m_id);
	writeAddress(address);
	m_wire.write(data);
	m_wire.endTransmission();
}

// NOTE: This doesn't do a waitForReady first, because it's not necessary to wait between
// multiple reads, and we want to use this to compose larger reads
uint8_t AT24C::readByte(uint16_t address)
{
	CZ_ASSERT(address<m_sizeBytes);

	m_wire.beginTransmission(m_id);
	writeAddress(address);
	m_wire.endTransmission();

	uint8_t ret = 0;
	m_wire.requestFrom(m_id, (uint8_t)1);
	if (m_wire.available())
	{
		ret = (uint8_t)m_wire.read();
	}
	return ret;
}

uint8_t AT24C::calcBulkSize(uint16_t address, uint16_t len)
{
	uint16_t pageOffset = address % m_pageSize;
	uint16_t leftInPage = m_pageSize - pageOffset;
	uint16_t todo = min(leftInPage, len);

	// The Wire library has a buffer size limit of BUFFER_LENGTH, which is 32 by default.
	// BUT because when we do a read or write to the eeprom we need to write 2 bytes first (the mem address to use),
	// we can only write/read a maximum of BUFFER_LENGTH - 2
	return (uint8_t)min(todo, BUFFER_LENGTH-2);
}

/**
 * AT24C32
 */
AT24C32::AT24C32(uint8_t address, TwoWire& wire)
	: AT24C(address, 128, 32, wire)
{
}

/**
 * AT24C256
 */
AT24C256::AT24C256(uint8_t address, TwoWire& wire)
	: AT24C(address, 512, 64, wire)
{
}

	bool test8(AT24C &mem)
	{
		CZ_LOG(logDefault, Log, F("Testing writting all in 8 bits"));
		{
			unsigned long t1 = micros();
			for (uint16_t addr = 0; addr < mem.getSizeBytes(); addr++)
			{
				mem.write8(addr, addr & 0xFF);
			}
			unsigned long t2 = micros();
			CZ_LOG(logDefault, Log, F("	Time to write all: %ld microseconds (%ld milliseconds)"), t2 - t1, (t2 - t1) / 1000);
		}

		{
			unsigned long t1 = micros();
			for (uint16_t addr = 0; addr < mem.getSizeBytes(); addr++)
			{
				uint8_t ret = mem.read8(addr);
				if (ret != (addr & 0xFF))
				{
					CZ_LOG(logDefault, Log, F("	Address %u - Expected %d, got %d"), (unsigned int)addr, (int)addr & 0xFF, ret);
					return false;
				}
			}
			unsigned long t2 = micros();
			CZ_LOG(logDefault, Log, F("	Time to real all: %ld microseconds (%ld milliseconds)"), t2 - t1, (t2 - t1) / 1000);
		}

		return true;
	}

	bool test16(AT24C &mem)
	{
		CZ_LOG(logDefault, Log, F("Testing writting all in 16 bits"));
		{
			unsigned long t1 = micros();
			for (uint16_t addr = 0; addr < mem.getSizeBytes(); addr += 2)
			{
				mem.write16(addr, addr);
			}
			unsigned long t2 = micros();
			CZ_LOG(logDefault, Log, F("	Time to write all: %ld microseconds (%ld milliseconds)"), t2 - t1, (t2 - t1) / 1000);
		}

		{
			unsigned long t1 = micros();
			for (uint16_t addr = 0; addr < mem.getSizeBytes(); addr += 2)
			{
				uint16_t ret = mem.read16(addr);
				if (ret != addr)
				{
					CZ_LOG(logDefault, Log, F("	Address %u - Expected %d, got %d"), (unsigned int)addr, (int)addr, ret);
					return false;
				}
			}
			unsigned long t2 = micros();
			CZ_LOG(logDefault, Log, F("	Time to real all: %ld microseconds (%ld milliseconds)"), t2 - t1, (t2 - t1) / 1000);
		}

		return true;
	}

	bool test32(AT24C &mem)
	{
		CZ_LOG(logDefault, Log, F("Testing writting all in 32 bits"));
		{
			unsigned long t1 = micros();
			for (uint16_t addr = 0; addr < mem.getSizeBytes(); addr += 4)
			{
				mem.write32(addr, (uint32_t(1) << 24) | (uint32_t(2) << 16) | addr);
			}
			unsigned long t2 = micros();
			CZ_LOG(logDefault, Log, F("	Time to write all: %ld microseconds (%ld milliseconds)"), t2 - t1, (t2 - t1) / 1000);
		}

		{
			unsigned long t1 = micros();
			for (uint16_t addr = 0; addr < mem.getSizeBytes(); addr += 4)
			{
				uint32_t ret = mem.read32(addr);
				uint32_t expected = (uint32_t(1) << 24) | (uint32_t(2) << 16) | addr;
				if (ret != expected)
				{
					CZ_LOG(logDefault, Log, F("	Address %u - Expected %lu, got %d"), (unsigned int)addr, (unsigned long)expected, ret);
					return false;
				}
			}
			unsigned long t2 = micros();
			CZ_LOG(logDefault, Log, F("	Time to real all: %ld microseconds (%ld milliseconds)"), t2 - t1, (t2 - t1) / 1000);
		}

		return true;
	}

	bool testBuffers(AT24C &mem, const char* buf)
	{
		static char readBuf[128];
		uint16_t bufSize = strlen(buf);

		CZ_LOG(logDefault, Log, F("Testing writting buffers of size %u"), bufSize);

		{
			unsigned long t1 = micros();
			for (uint16_t addr = 0; addr < mem.getSizeBytes(); addr += bufSize)
			{
				uint16_t todo = min(bufSize, mem.getSizeBytes() - addr);
				auto ret = mem.write(addr, (const uint8_t *)buf, todo);
				CZ_ASSERT(ret == todo);
			}
			unsigned long t2 = micros();
			CZ_LOG(logDefault, Log, F("	Time to write all: %ld microseconds (%ld milliseconds)"), t2 - t1, (t2 - t1) / 1000);
		}

		{
			unsigned long t1 = micros();
			for (uint16_t addr = 0; addr < mem.getSizeBytes(); addr += bufSize)
			{
				uint16_t todo = min(bufSize, mem.getSizeBytes() - addr);
				memset(readBuf, 0, 128);
				auto ret = mem.read(addr, (uint8_t *)readBuf, todo);
				readBuf[ret] = 0;
				CZ_ASSERT(ret == todo);
				if (strncmp(buf, readBuf, todo) != 0)
				{
					CZ_LOG(logDefault, Log, F("	Address %u - Expected %s, got %s"), (unsigned int)addr, buf, readBuf);
					return false;
				}
				CZ_LOG(logDefault, Log, F("%s"), readBuf);
			}
			unsigned long t2 = micros();
			CZ_LOG(logDefault, Log, F("	Time to read all: %ld microseconds (%ld milliseconds)"), t2 - t1, (t2 - t1) / 1000);
		}

		return true;
	}

	bool readAndCompare(AT24C& mem, uint16_t addr, const char* expected, uint16_t len)
	{
		char readBuf[128];
		if (len>128)
		{
			return false;
		}

		uint16_t todo = min(len, mem.getSizeBytes() - addr);
		memset(readBuf, 0, 128);
		auto ret = mem.read(addr, (uint8_t *)readBuf, todo);
		readBuf[ret] = 0;
		CZ_ASSERT(ret == todo);
		if (strncmp(expected, readBuf, todo) != 0)
		{
			CZ_LOG(logDefault, Log, F("	Address %u - Expected %s, got %s"), (unsigned int)addr, expected, readBuf);
			return false;
		}
		CZ_LOG(logDefault, Log, F("%s"), readBuf);
		return true;
	}

	// #RVF remove this
	bool tempTestBuffers(AT24C &mem, const char* buf)
	{
		uint16_t bufSize = strlen(buf);

		CZ_LOG(logDefault, Log, F("Testing writting buffers of size %u"), bufSize);

		{
			unsigned long t1 = micros();
			for (uint16_t addr = 0; addr < mem.getSizeBytes(); addr += bufSize)
			{
				uint16_t todo = min(bufSize, mem.getSizeBytes() - addr);
				auto ret = mem.write(addr, (const uint8_t *)buf, todo);
				CZ_ASSERT(ret == todo);

				if (!readAndCompare(mem, addr, buf, todo))
				{
					CZ_LOG(logDefault, Log, F("Test failed"));
					return false;
				}
			}
			unsigned long t2 = micros();
			CZ_LOG(logDefault, Log, F("	Time to write all: %ld microseconds (%ld milliseconds)"), t2 - t1, (t2 - t1) / 1000);
		}

		return true;
	}

void printFirst64Bits(AT24C& mem)
{
	uint8_t data[8];
	for(int i=0; i<8; i++)
		data[i] = mem.read8(i);

	CZ_LOG(logDefault, Log, F("First 8 bytes = {%d,%d,%d,%d,%d,%d,%d,%d}"),
		(int)data[0], (int)data[1], (int)data[2], (int)data[3],
		(int)data[4], (int)data[5], (int)data[6], (int)data[7]);
}


bool runTests(AT24C& mem)
{
	CZ_LOG(logDefault, Log, F("Starting tests..."));

#if 1
	printFirst64Bits(mem);
	if (!test8(mem)) return false;
	printFirst64Bits(mem);
	if (!test16(mem)) return false;
	printFirst64Bits(mem);
	if (!test32(mem)) return false;
	printFirst64Bits(mem);

	static const char *buf1 = "Rui_Miguel_Valente_Figueira";
	// slightly more than 32 bytes, to test across 32-bytes page boundaries
	static const char *buf2 = "Rui_Miguel_Valente_Figueira_Hello";
	// slightly more than 63 bytes, to test across 64-bytes page boundaries
	static const char *buf3 = "Rui_Miguel_Valente_Figueira_Hello. Hey, diddle, diddle, the cat a";

	if (!testBuffers(mem, buf1)) return false;
	printFirst64Bits(mem);
	if (!testBuffers(mem, buf2)) return false;
	printFirst64Bits(mem);
	if (!testBuffers(mem, buf3)) return false;
	printFirst64Bits(mem);
	CZ_LOG(logDefault, Log, F("Tests finished"));
#else
	CZ_LOG(logDefault, Log, F("Starting tests..."));
	static const char *buf1 = "Rui_Miguel_Valente_Figueira";
	if (!tempTestBuffers(mem, buf1)) return false;
	CZ_LOG(logDefault, Log, F("Tests finished"));
#endif

	return true;
}


} // namespace cz