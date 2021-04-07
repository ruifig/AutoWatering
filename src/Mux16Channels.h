#pragma once

#include "Utils.h"
#include "MCP23017Wrapper.h"

namespace cz
{

class Mux16Channels
{
public:
    Mux16Channels(MCP23017Wrapper& ioExpander, IOExpanderPin s0, IOExpanderPin s1, IOExpanderPin s2, IOExpanderPin s3, ArduinoPin zPin);

    void begin();
    int read(MultiplexerPin channel) const;

private:
    
    MCP23017Wrapper& m_ioExpander;
    IOExpanderPin m_sPins[4];
    ArduinoPin m_zPin; // Arduino pin we use to send/receive data
};


} // namespace cz
