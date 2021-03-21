#include "Display.h"

namespace cz
{

Display::Display(Adafruit_RGBLCDShield &lcd, ProgramData& data)
    : m_lcd(lcd)
    , m_data(data)
{
}


float Display::tick(float deltaSeconds)
{
    return 1.0f/20.0f;
}

} // namespace cz