#pragma once

#include <Adafruit_RGBLCDShield.h>
#include "ProgramData.h"

namespace cz
{

class Display
{
public:
    Display(Adafruit_RGBLCDShield& lcd, ProgramData& data);
    Display(const Display&) = delete;
    Display& operator=(const Display&) = delete;
    float tick(float deltaSeconds);
private:
    Adafruit_RGBLCDShield& m_lcd;
    ProgramData& m_data;
};

} // namespace cz