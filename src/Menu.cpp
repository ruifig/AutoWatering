#include "Menu.h"
#include "DisplayCommon.h"
#include "gfx/MyDisplay1.h"

namespace cz
{

extern MyDisplay1 gScreen;

void Menu::show()
{
}

void Menu::clearEntireArea()
{
	fillRect(LayoutHelper::getMenuFullArea(), AW_SCREEN_BKG_COLOUR);
}

void Menu::hide()
{
	clearEntireArea();
}

} // namespace cz
