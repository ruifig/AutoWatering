/**
 * Wrapper for the MCP23S17 class, to only access pin values with the strong type
 */

#pragma once

#include <MCP23S17.h>
#include "Config.h"
#include <utility>

namespace cz
{

class MCP23S17Wrapper
{
    public:

        MCP23S17Wrapper(SPIClass *spi, ArduinoPin cs, uint8_t addr) : m_inner(spi, cs.raw, addr) {}
        MCP23S17Wrapper(SPIClass &spi, ArduinoPin cs, uint8_t addr) : m_inner(spi, cs.raw, addr) {}

        void begin(){ m_inner.begin(); }
        void pinMode(IOExpanderPin pin, uint8_t mode) { m_inner.pinMode(pin.raw, mode); }
        void digitalWrite(IOExpanderPin pin, uint8_t value) { m_inner.digitalWrite(pin.raw, value); }
        uint8_t digitalRead(IOExpanderPin pin) { return m_inner.digitalRead(pin.raw); }

        uint8_t readPort(uint8_t port) { return m_inner.readPort(port); }
        uint16_t readPort() { return m_inner.readPort(); }
        void writePort(uint8_t port, uint8_t val) { m_inner.writePort(port, val); }
        void writePort(uint16_t val) { m_inner.writePort(val); }
        void enableInterrupt(IOExpanderPin pin, uint8_t type) { m_inner.enableInterrupt(pin.raw, type); }
        void disableInterrupt(IOExpanderPin pin) { m_inner.disableInterrupt(pin.raw); }
        void setMirror(boolean m) { m_inner.setMirror(m); }
        uint16_t getInterruptPins() { return m_inner.getInterruptPins(); }
        uint16_t getInterruptValue() { return m_inner.getInterruptValue(); }
        void setInterruptLevel(uint8_t level) { m_inner.setInterruptLevel(level); }
        void setInterruptOD(boolean openDrain) { m_inner.setInterruptOD(openDrain); }

    private:

    MCP23S17 m_inner;
};

} // namespace cz
