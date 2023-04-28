#include "ShotConfirmationMenu.h"
#include "DisplayCommon.h"
#include "Icons.h"
#include <crazygaze/micromuc/Profiler.h>

void doGroupShot(uint8_t index);

namespace cz
{

using namespace gfx;

namespace 
{
	const LabelData labelData_confirmation PROGMEM =
	{
		LayoutHelper::getMenuButtonRect(1, 1),
		HAlign::Center, VAlign::Center,
		TINY_FONT,
		AW_GRAPH_VALUES_TEXT_COLOUR, AW_SCREEN_BKG_COLOUR,
		WidgetFlag::EraseBkg
	};
}

ShotConfirmationMenu::ShotConfirmationMenu()
	: m_confirmationLabel(&labelData_confirmation, F("Sure?"))
{
}

ShotConfirmationMenu::~ShotConfirmationMenu()
{
}

void ShotConfirmationMenu::init()
{
	m_buttons[(int)ButtonId::Yes].init((int)ButtonId::Yes, LayoutHelper::getMenuButtonPos(2,1), AW_SCREEN_BKG_COLOUR, img_SetThreshold);
	m_buttons[(int)ButtonId::No].init((int)ButtonId::No, LayoutHelper::getMenuButtonPos(3,1), AW_SCREEN_BKG_COLOUR, img_Close);
}

void ShotConfirmationMenu::tick(float deltaSeconds)
{
	PROFILE_SCOPE(F("ShotConfirmationMenu"));

	draw();
}

void ShotConfirmationMenu::onEvent(const Event& evt)
{
}

bool ShotConfirmationMenu::processTouch(const Pos& pos)
{
	auto checkButtonPressed = [&](ButtonId id) -> bool
	{
		ImageButton& btn = m_buttons[(int)id];
		if (btn.canAcceptInput() && btn.contains(pos))
		{
			return true;
		}
		else
		{
			return false;
		}
	};

	if (checkButtonPressed(ButtonId::Yes))
	{
		CZ_ASSERT(gCtx.data.hasGroupSelected());
		GroupData* data = gCtx.data.getSelectedGroup();
		doGroupShot(data->getIndex());
		m_close = true;
	}
	else if (checkButtonPressed(ButtonId::No))
	{
		m_close = true;
	};

	return false;
}

void ShotConfirmationMenu::show()
{
	Menu::show();

	m_close = false;
	m_confirmationLabel.setDirty(true);
	for(auto&& b : m_buttons)
	{
		b.setEnabled(true);
		b.setVisible(true);
	}
}

void ShotConfirmationMenu::hide()
{
	for(auto&& b : m_buttons)
	{
		b.setEnabled(false);
		b.setVisible(false);
	}

	Menu::hide();
}

bool ShotConfirmationMenu::checkClose()
{
	return m_close;
}

void ShotConfirmationMenu::draw()
{
	m_confirmationLabel.draw();

	for(auto&& b : m_buttons)
	{
		b.draw();
	}
}

} // namespace cz
