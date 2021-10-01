#include "Button.h"

namespace cz::gfx
{

#pragma region BaseButton
//////////////////////////////////////////////////////////////////////////
// BaseButton
//////////////////////////////////////////////////////////////////////////

void BaseButton::initHelper(uint8_t id, Adafruit_GFX &gfx, const Rect& pos, uint16_t bkgColour)
{
	m_id = id;
	m_gfx = &gfx;
	m_pos = pos;
	m_bkgColour = bkgColour;
	m_currState = ButtonState::Released;
	m_lastState = ButtonState::Released;
	m_needsRedraw = true;
}

bool BaseButton::contains(int16_t x, int16_t y) const
{
	return m_pos.contains(x, y);
}

void BaseButton::setState(ButtonState state)
{
	if (state != m_currState)
	{
		m_needsRedraw = true;
	}

	m_lastState = m_currState;
	m_currState = state;
}

bool BaseButton::justReleased() const
{
	return m_currState==ButtonState::Released && m_lastState == ButtonState::Pressed;
}

#pragma endregion


#pragma region ImageButton
//////////////////////////////////////////////////////////////////////////
// ImageButton
//////////////////////////////////////////////////////////////////////////

ImageButton::ImageButton()
{
}

void ImageButton::init(uint8_t id, Adafruit_GFX &gfx, const Pos& pos, uint16_t bkgColour, const Image& img)
{
	initHelper(id, gfx, {pos.x, pos.y, img.width, img.height}, bkgColour);
	m_img = img;
}

void ImageButton::draw(bool forceDraw)
{
	if (!(m_needsRedraw || forceDraw))
	{
		return;
	}

	if (m_currState == ButtonState::Hidden)
	{
		 // do nothing;
	}
	else if (m_currState == ButtonState::Disabled)
	{
		drawRGBBitmapDisabled_P(m_pos.x, m_pos.y, m_img.bmp, m_img.mask, m_img.width, m_img.height, Colour_Black);
	}
	else
	{
		drawRGBBitmap_P(m_pos.x, m_pos.y, m_img.bmp, m_img.mask, m_img.width, m_img.height, Colour_Black);
	}

	m_needsRedraw = false;
}

#pragma endregion


#pragma region TextButton
//////////////////////////////////////////////////////////////////////////
// TextButton
//////////////////////////////////////////////////////////////////////////

void TextButton::init(uint8_t id, Adafruit_GFX &gfx, const GFXfont* font, const Rect& pos, uint16_t bkgColour, uint16_t outlineColour, uint16_t textColour, const char* text, uint8_t textMagnification)
{
	BaseButton::initHelper(id, gfx, pos, bkgColour);
	m_outlineColour = outlineColour;
	m_textColour = textColour;
	m_textMagX = textMagnification;
	m_textMagY = textMagnification;
	m_font = font;
	strncpy(m_text, text, sizeof(m_text));
	m_text[sizeof(m_text)-1] = 0;
}

void TextButton::draw(bool forceDraw)
{
	// #RVF : Make use of forceDraw
	
	uint16_t fillColour, textColour;
	int16_t  text_x, text_y;  // position of the text outline
	uint16_t text_w, text_h;  // size of the text outline

	if (m_inverted)
	{
		fillColour = m_textColour;
		textColour = m_bkgColour;
	}
	else
	{
		fillColour = m_bkgColour;
		textColour = m_textColour;
	}
	
	uint8_t r = min(m_pos.width, m_pos.height) / 4; // Corner radius
	m_gfx->fillRoundRect(m_pos.x, m_pos.y, m_pos.width, m_pos.height, r, fillColour);
	m_gfx->drawRoundRect(m_pos.x, m_pos.y, m_pos.width, m_pos.height, r, m_outlineColour);

	// set the button's text size first, it 's used by getTextBounds()
	m_gfx->setTextSize(m_textMagX, m_textMagY);

	m_gfx->setFont(m_font);

	// if a custom font is used, calculate the text cursor from the
	// font's data and the string
	if(m_gfx->getGfxFont())
	{
		// give the string and virtual cursor and get the enclosing rectangle
		m_gfx->getTextBounds(m_text, m_pos.x, m_pos.y+m_pos.height,
		&text_x, &text_y, &text_w, &text_h);
		// with this rectangle set the cursor to center the text in the button
		m_gfx->setCursor((m_pos.x + ((m_pos.width - text_w) / 2) - 1), (m_pos.y + ((m_pos.height + text_h) / 2)));
	}
	else
	{
		// Default font
		m_gfx->setCursor(m_pos.x + (m_pos.width/2) - (strlen(m_text) * 3 * m_textMagX),
		m_pos.y + (m_pos.height/2) - (4 * m_textMagY));
	}
	m_gfx->setTextColor(textColour);
	m_gfx->print(m_text);
}

#if 0
void TextButton::drawButton(boolean inverted)
{
	uint16_t fill, outline, text;
	int16_t  text_x, text_y;  // position of the text outline
	uint16_t text_w, text_h;  // size of the text outline
	
	if (!inverted) {
		fill = _fillcolor;
		outline = _outlinecolor;
		text = _textcolor;
		} else {
		fill = _textcolor;
		outline = _outlinecolor;
		text = _fillcolor;
	}

	uint8_t r = min(_w, _h) / 4; // Corner radius
	_gfx->fillRoundRect(_x1, _y1, _w, _h, r, fill);
	_gfx->drawRoundRect(_x1, _y1, _w, _h, r, outline);

	// set the button's text size first, it 's used by getTextBounds()
	_gfx->setTextSize(_textsize_x, _textsize_y);
	// if a custom font is used, calculate the text cursor from the
	// font's data and the string
	if(_gfx->getGfxFont()) {
		// give the string and virtual cursor and get the enclosing rectangle
		_gfx->getTextBounds(_label, _x1, _y1+_h,
		&text_x, &text_y, &text_w, &text_h);
		// with this rectangle set the cursor to center the text in the button
		_gfx->setCursor((_x1 + ((_w - text_w) / 2) - 1), (_y1 + ((_h + text_h) / 2)));
		} else { // Default font
		_gfx->setCursor(_x1 + (_w/2) - (strlen(_label) * 3 * _textsize_x),
		_y1 + (_h/2) - (4 * _textsize_y));
	}
	_gfx->setTextColor(text);
	_gfx->print(_label);
}
#endif


#pragma endregion

} // namespace cz::gfx
