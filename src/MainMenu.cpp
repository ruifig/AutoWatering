#include "MainMenu.h"
#include "DisplayCommon.h"
#include "Icons.h"

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

	initButton(ButtonId::StartGroup, LayoutHelper::getMenuButtonPos(0,0), SCREEN_BKG_COLOUR, img_Play);
	initButton(ButtonId::StopGroup, LayoutHelper::getMenuButtonPos(0,0), SCREEN_BKG_COLOUR, img_Stop);
	initButton(ButtonId::Shot, LayoutHelper::getMenuButtonPos(1,0), SCREEN_BKG_COLOUR, img_Shot);
	initButton(ButtonId::Settings, LayoutHelper::getMenuButtonPos(2,0), SCREEN_BKG_COLOUR, img_Settings);

	show();
}

void MainMenu::tick(float deltaSeconds)
{
	draw();
}

bool MainMenu::processTouch(const Pos& pos)
{
	{
		ImageButton& btn = m_buttons[(int)ButtonId::StartGroup];
		if (btn.canAcceptInput() && btn.contains(pos))
		{
			CZ_ASSERT(gCtx.data.hasGroupSelected());
			gCtx.data.getSelectedGroup()->setRunning(true);
			gCtx.data.saveGroupConfig(gCtx.data.getSelectedGroupIndex());
			return true;
		}
	}
	
	{
		ImageButton& btn = m_buttons[(int)ButtonId::StopGroup];
		if (btn.canAcceptInput() && btn.contains(pos))
		{
			CZ_ASSERT(gCtx.data.hasGroupSelected());
			gCtx.data.getSelectedGroup()->setRunning(false);
			gCtx.data.saveGroupConfig(gCtx.data.getSelectedGroupIndex());
			return true;
		}
	}

	{
		ImageButton& btn = m_buttons[(int)ButtonId::Shot];
		if (btn.canAcceptInput() && btn.contains(pos))
		{
			m_buttonPressed = ButtonId::Shot;
			return true;
		}
	}
	
	{
		ImageButton& btn = m_buttons[(int)ButtonId::Settings];
		if (btn.canAcceptInput() && btn.contains(pos))
		{
			CZ_ASSERT(gCtx.data.hasGroupSelected());
			m_buttonPressed = ButtonId::Settings;
			return true;
		}
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
