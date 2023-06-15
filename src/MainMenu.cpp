#include "MainMenu.h"
#include "DisplayCommon.h"
#include "Icons.h"
#include <crazygaze/micromuc/Profiler.h>

namespace cz
{

using namespace gfx;

MainMenu::MainMenu()
{
}

MainMenu::~MainMenu()
{
}

void MainMenu::init()
{
	m_buttonPressed = ButtonId::Max;
	
	auto initButton = [this](ButtonId id, auto&&... params)
	{
		m_buttons[(int)id].init((int)id, std::forward<decltype(params)>(params)...);
		m_buttons[(int)id].setClearWhenHidden(false);
	};

	initButton(ButtonId::StartGroup, LayoutHelper::getMenuButtonPos(0,0), AW_TOUCHUI_SCREEN_BKG_COLOUR, img_Play);
	initButton(ButtonId::StopGroup, LayoutHelper::getMenuButtonPos(0,0), AW_TOUCHUI_SCREEN_BKG_COLOUR, img_Stop);
	initButton(ButtonId::Shot, LayoutHelper::getMenuButtonPos(1,0), AW_TOUCHUI_SCREEN_BKG_COLOUR, img_Shot);
	initButton(ButtonId::Settings, LayoutHelper::getMenuButtonPos(2,0), AW_TOUCHUI_SCREEN_BKG_COLOUR, img_Settings);
	initButton(ButtonId::Up, LayoutHelper::getMenuButtonPos(8,0), AW_TOUCHUI_SCREEN_BKG_COLOUR, img_Up);
	initButton(ButtonId::Down, LayoutHelper::getMenuButtonPos(8,1), AW_TOUCHUI_SCREEN_BKG_COLOUR, img_Down);

	show();
}

void MainMenu::tick(float deltaSeconds)
{
	PROFILE_SCOPE(F("MainMenu"));

	draw();
}

bool MainMenu::processTouch(const Pos& pos)
{
	auto testButtonPressed = [this](const Pos& pos) -> ButtonId
	{
		for(int i=0; i<(int)ButtonId::Max; i++)
		{
			ImageButton& btn = m_buttons[i];
			if (btn.canAcceptInput() && btn.contains(pos))
			{
				return (ButtonId)i;
			}
		}
		return ButtonId::Max;
	};

	m_buttonPressed = testButtonPressed(pos);

	if (m_buttonPressed==ButtonId::StartGroup || m_buttonPressed==ButtonId::StopGroup)
	{
		CZ_ASSERT(gCtx.data.hasGroupSelected());
		gCtx.data.getSelectedGroup()->setRunning(m_buttonPressed==ButtonId::StartGroup ? true : false);
		gCtx.data.saveGroupConfig(gCtx.data.getSelectedGroupIndex());
		return true;
	}
	else if (m_buttonPressed==ButtonId::Shot)
	{
		CZ_ASSERT(gCtx.data.hasGroupSelected());
		// Do nothing. m_buttonPressed is set, and the external code will detect that
		return true;
	}
	else if (m_buttonPressed==ButtonId::Settings)
	{
		CZ_ASSERT(gCtx.data.hasGroupSelected());
		// Do nothing. m_buttonPressed is set, and the external code will detect that
		return true;
	}
	else if (m_buttonPressed==ButtonId::Up || m_buttonPressed==ButtonId::Down)
	{
		// Do nothing. m_buttonPressed is set, and the external code will detect that
		return true;
	}

	return false;
}

void MainMenu::updateButtons()
{
	bool isGroupSelected = gCtx.data.hasGroupSelected();
	bool groupIsRunning = isGroupSelected ? gCtx.data.getSelectedGroup()->isRunning() : false;

	m_buttons[(int)ButtonId::StartGroup].setVisible(!(isGroupSelected && groupIsRunning));
	m_buttons[(int)ButtonId::StartGroup].setEnabled(isGroupSelected && !groupIsRunning);

	m_buttons[(int)ButtonId::StopGroup].setVisible(isGroupSelected && groupIsRunning);

	// Shot and Settings stay enabled even if the group is not running. This is intentional
	m_buttons[(int)ButtonId::Shot].setEnabled(isGroupSelected);
	m_buttons[(int)ButtonId::Settings].setEnabled(isGroupSelected);

	m_buttons[(int)ButtonId::Up].setEnabled(true);
	m_buttons[(int)ButtonId::Down].setEnabled(true);

}

void MainMenu::onEvent(const Event& evt)
{
	switch (evt.type)
	{
		case Event::GroupSelected:
		case Event::GroupOnOff:
		case Event::ConfigLoad:
		{
			updateButtons();
		}
		break;
	}
}

void MainMenu::draw()
{
	for(gfx::ImageButton& btn : m_buttons)
	{
		btn.draw(m_forceDraw);
	}
	m_forceDraw = false;
}

void MainMenu::show()
{
	Menu::show();

	for(gfx::ImageButton& btn : m_buttons)
	{
		btn.setVisible(true);
	}

	updateButtons();
}

void MainMenu::hide()
{
	for(gfx::ImageButton& btn : m_buttons)
	{
		btn.setVisible(false);
	}

	Menu::hide();
}

MainMenu::ButtonId MainMenu::checkButtonPressed()
{
	ButtonId ret = m_buttonPressed;
	m_buttonPressed = ButtonId::Max;
	return ret;
}


bool MainMenu::checkClose()
{
	return false;
}

} // namespace cz
