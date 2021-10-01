#pragma once

#include "Widget.h"

namespace cz::gfx
{

enum class ButtonState : uint8_t
{
	Hidden,
	Disabled,
	Pressed,
	Released
};

class BaseButton : public Widget
{
  public:
	using Widget::Widget;

	bool contains(int16_t x, int16_t y) const;
	bool isDisabled() const
	{
		return m_currState == ButtonState::Disabled;
	}

	void setState(ButtonState state);
	bool justReleased() const;

  protected:	

	void initHelper(uint8_t id, Adafruit_GFX &gfx, const Rect& pos, uint16_t bkgColour);
	
	uint8_t m_id;
	Adafruit_GFX* m_gfx;
	Rect m_pos;
	uint16_t m_bkgColour;
	ButtonState m_lastState;
	ButtonState m_currState;
	bool m_needsRedraw;
};


/**
 * Image button, with the used bitmap being in PROGMEM
 */
class ImageButton : public BaseButton
{
  public:
	ImageButton();
	void init(uint8_t id, Adafruit_GFX &gfx, const Pos& pos, uint16_t bkgColour, const Image& img);
	// Widget
	virtual void draw(bool forceDraw = false) override;
	//

  protected:
	Image m_img;
};


/**
 * Button that displays a text instead of an image 
 */
class TextButton : public BaseButton
{
  public:
	void init(uint8_t id, Adafruit_GFX &gfx, const GFXfont* font, const Rect& pos, uint16_t bkgColour, uint16_t outlineColour, uint16_t textColour, const char* text, uint8_t textMagnification = 1);

	void setInverted(bool inverted)
	{
		m_inverted = inverted;
	}
	virtual void draw(bool forceDraw = false) override;

  private:
	bool m_inverted = false;
	uint16_t m_outlineColour;
	uint16_t m_textColour;

	// Text magnification
	uint8_t m_textMagX;
	uint8_t m_textMagY;

	const GFXfont* m_font = nullptr;
	char m_text[10];
};

} // namespace cz::gfx

