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
	bool contains(const Pos& pos) const;
	bool canAcceptInput() const
	{
		return m_visible && m_enabled;
	}

	void setEnabled(bool enabled);
	void setVisible(bool visible);

	void setPressed(bool pressed);
	bool justReleased() const;

	void setClearWhenHidden(bool doClear);
  protected:	

	void initHelper(uint8_t id, const Rect& pos, Colour bkgColour);
	
	uint8_t m_id;
	Rect m_pos;
	Colour m_bkgColour;

	bool m_needsRedraw : 1;
	bool m_visible : 1;
	bool m_enabled : 1;
	bool m_pressed : 1;
	bool m_lastPressed : 1;
	bool m_clearWhenHidden : 1;
};


/**
 * Image button, with the used bitmap being in PROGMEM
 */
class ImageButton : public BaseButton
{
  public:
	ImageButton();
	void init(uint8_t id, const Pos& pos, Colour bkgColour, const Image& img);
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
	void init(uint8_t id, const GFXfont* font, const Rect& pos, Colour bkgColour, Colour outlineColour, Colour textColour, const char* text, uint8_t textMagnification = 1);

	void setInverted(bool inverted)
	{
		m_inverted = inverted;
	}
	virtual void draw(bool forceDraw = false) override;

  private:
	bool m_inverted = false;
	Colour m_outlineColour;
	Colour m_textColour;

	// Text magnification
	uint8_t m_textMagX;
	uint8_t m_textMagY;

	const GFXfont* m_font = nullptr;
	char m_text[10];
};

} // namespace cz::gfx

