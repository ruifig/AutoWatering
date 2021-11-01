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

	void setEnabled(bool enabled);
	void setVisible(bool visible);
	void setPressed(bool pressed);
	bool justReleased() const;

	void setClearWhenHidden(bool doClear);
  protected:	

	void initHelper(uint8_t id, const Rect& pos, uint16_t bkgColour);
	
	uint8_t m_id;
	Rect m_pos;
	uint16_t m_bkgColour;

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
	void init(uint8_t id, const Pos& pos, uint16_t bkgColour, const Image& img);
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
	void init(uint8_t id, const GFXfont* font, const Rect& pos, uint16_t bkgColour, uint16_t outlineColour, uint16_t textColour, const char* text, uint8_t textMagnification = 1);

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

