#pragma once

#include "Menu.h"
#include "gfx/Label.h"
#include "gfx/Button.h"

namespace cz
{

class ShotConfirmationMenu : public Menu
{
  public:

	ShotConfirmationMenu();
	virtual ~ShotConfirmationMenu();
	virtual void init() override;
	virtual void tick(float deltaSeconds) override;
	virtual void onEvent(const Event& evt) override;
	virtual bool processTouch(const Pos& pos) override;

	virtual void show() override;
	virtual void hide() override;
	virtual bool checkClose() override;

  protected:
	virtual void draw() override;

	enum class ButtonId : uint8_t
	{
		First,
		Yes = First,
		No,
		Max
	};

	gfx::FixedLabel<> m_confirmationLabel;
	gfx::ImageButton m_buttons[(int)ButtonId::Max];
	bool m_close = false;
};

} // namespace cz
