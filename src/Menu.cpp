#include "Menu.h"
#include "DisplayCommon.h"
#include "gfx/TFTeSPIWrapper.h"

namespace cz
{

void Menu::show()
{
}

void Menu::clearEntireArea()
{
	fillRect(LayoutHelper::getMenuFullArea(), AW_TOUCHUI_SCREEN_BKG_COLOUR);
}

void Menu::hide()
{
	clearEntireArea();
}

} // namespace cz
