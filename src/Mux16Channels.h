#pragma once

#include "Utils.h"
#include "MCP23S17Wrapper.h"

namespace cz
{

class Mux16Channels
{
public:
    Mux16Channels(MCP23S17Wrapper& ioExpander, IOExpanderPin s0, IOExpanderPin s1, IOExpanderPin s2, IOExpanderPin s3, ArduinoPin zPin)
        : m_ioExpander(ioExpander)
        , m_sPins({s0, s1, s2, s3})
        , m_zPin(zPin)
    {
    }

    void setup();
    int read(MultiplexerPin channel) const;

private:
    
    MCP23S17Wrapper& m_ioExpander;
    IOExpanderPin m_sPins[4];
    ArduinoPin m_zPin; // Arduino pin we use to send/receive data
};


} // namespace cz
