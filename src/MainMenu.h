#pragma once

#include "Menu.h"
#include "gfx/Button.h"

namespace cz
{

class MainMenu : public Menu
{
  public:
	MainMenu();
	virtual ~MainMenu();

	virtual void init() override;
	virtual void tick(float deltaSeconds) override;
	virtual void onEvent(const Event& evt) override;
	virtual bool processTouch(const Pos& pos) override;

	virtual void show() override;
	virtual void hide() override;
	virtual bool checkClose() override;

	enum class ButtonId : uint8_t
	{
		StartGroup,
		StopGroup,
		Shot,
		Settings,
		Max
	};

	ButtonId checkButtonPressed();

  private:

	virtual void draw() override;
	void updateButtons();

	gfx::ImageButton m_buttons[(int)ButtonId::Max];
	ButtonId m_buttonPressed = ButtonId::Max;

};

} // namespace cz
