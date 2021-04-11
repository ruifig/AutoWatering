#pragma once

#include <Arduino.h>
#include <Wire.h>

/**
 * The AT24CX chips have a possible ic2 address of 0x50..0x57
 * 
 * 
 * AT24C32	32kbits	  32 bytes pages
 * AT24C64	64kbits	  32 bytes pages
 * AT24C128	128kbits	64 bytes pages
 * AT24C256	256kbits	64 bytes pages
 */
namespace cz
{

class AT24C
{
  public:
    AT24C(uint8_t address, uint8_t pageSize, uint16_t sizeBytes, TwoWire& wire);
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

  private:
};

bool runTests(AT24C& mem);


} // namespace cz

