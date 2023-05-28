#include "Button.h"
#include "TFTeSPIWrapper.h"

namespace cz::gfx
{

//////////////////////////////////////////////////////////////////////////
// BaseButton
//////////////////////////////////////////////////////////////////////////

void BaseButton::initHelper(uint8_t id, const Rect& pos, Colour bkgColour)
{
	m_id = id;
	m_pos = pos;
	m_bkgColour = bkgColour;
	m_needsRedraw = true;
	m_visible = true;
	m_enabled = true;
	m_pressed = false;
	m_lastPressed = false;
	m_clearWhenHidden = true;
}

bool BaseButton::contains(int16_t x, int16_t y) const
{
	return m_pos.contains(x, y);
}

bool BaseButton::contains(const Pos& pos) const
{
	return m_pos.contains(pos);
}

bool BaseButton::justReleased() const
{
	return m_pressed == false && m_lastPressed == true;
}

void BaseButton::setEnabled(bool enabled)
{
	if (m_enabled == enabled)
	{
		return;
	}

	m_enabled = enabled;
	m_needsRedraw = true;
	if (!enabled)
	{
		m_pressed = false;
	}
}

void BaseButton::setVisible(bool visible)
{
	if (m_visible == visible)
	{
		return;
	}

	m_visible = visible;
	m_needsRedraw = true;
	if (!visible)
	{
		m_pressed = false;
	}
}

void BaseButton::setPressed(bool pressed)
{
	if (m_visible && m_enabled)
	{
		m_pressed = pressed;
		m_needsRedraw = true;
	}
}

void BaseButton::setClearWhenHidden(bool doClear)
{
	if (m_clearWhenHidden!=doClear)
	{
		m_clearWhenHidden = doClear;
		if (doClear && !m_visible)
		{
			m_needsRedraw = true;
		}
	}
}

void BaseButton::move(int16_t x, int16_t y, bool eraseCurrentPosition)
{
	if (eraseCurrentPosition)
	{
		if (m_enabled)
		{
			fillRect(m_pos, AW_SCREEN_BKG_COLOUR );
		}
	}

	m_pos.x = x;
	m_pos.y = y;
	m_needsRedraw = true;
}

//////////////////////////////////////////////////////////////////////////
// ImageButton
//////////////////////////////////////////////////////////////////////////

ImageButton::ImageButton()
{
}

void ImageButton::init(uint8_t id, const Pos& pos, Colour bkgColour, const Image& img)
{
	initHelper(id, {pos.x, pos.y, img.width, img.height}, bkgColour);
	m_img = img;
}

void ImageButton::draw(bool forceDraw)
{
	if (!(m_needsRedraw || forceDraw))
	{
		return;
	}

	if (!m_visible)
	{
		if (m_clearWhenHidden)
		{
			fillRect(m_pos, AW_SCREEN_BKG_COLOUR );
		}
	}
	else if (m_enabled)
	{
		drawRGBBitmap_P(m_pos.x, m_pos.y, m_img.bmp, m_img.mask, m_img.width, m_img.height, AW_SCREEN_BKG_COLOUR);
	}
	else
	{
		drawRGBBitmapDisabled_P(m_pos.x, m_pos.y, m_img.bmp, m_img.mask, m_img.width, m_img.height, AW_SCREEN_BKG_COLOUR);
	}

	m_needsRedraw = false;
}

//////////////////////////////////////////////////////////////////////////
// TextButton
//////////////////////////////////////////////////////////////////////////

void TextButton::init(uint8_t id, const GFXfont* font, const Rect& pos, Colour bkgColour, Colour outlineColour, Colour textColour, const char* text, uint8_t textMagnification)
{
	BaseButton::initHelper(id, pos, bkgColour);
	m_outlineColour = outlineColour;
	m_textColour = textColour;
	m_textMag = textMagnification;
	m_font = font;
	strncpy(m_text, text, sizeof(m_text));
	m_text[sizeof(m_text)-1] = 0;
}

void TextButton::draw(bool forceDraw)
{
	// #RVF : Make use of forceDraw
	
	Colour fillColour, textColour;
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
	TFTeSPIWrapper::getInstance()->fillRoundRect(m_pos.x, m_pos.y, m_pos.width, m_pos.height, r, fillColour);
	TFTeSPIWrapper::getInstance()->drawRoundRect(m_pos.x, m_pos.y, m_pos.width, m_pos.height, r, m_outlineColour);

	// set the button's text size first, it 's used by getTextBounds()
	TFTeSPIWrapper::getInstance()->setTextSize(m_textMag);

	TFTeSPIWrapper::getInstance()->setFont(m_font);

	// if a custom font is used, calculate the text cursor from the
	// font's data and the string
	if(TFTeSPIWrapper::getInstance()->getGfxFont())
	{
		// give the string and virtual cursor and get the enclosing rectangle
		TFTeSPIWrapper::getInstance()->getTextBounds(m_text, m_pos.x, m_pos.y+m_pos.height,
		&text_x, &text_y, &text_w, &text_h);
		// with this rectangle set the cursor to center the text in the button
		TFTeSPIWrapper::getInstance()->setCursor((m_pos.x + ((m_pos.width - text_w) / 2) - 1), (m_pos.y + ((m_pos.height + text_h) / 2)));
	}
	else
	{
		// Default font
		TFTeSPIWrapper::getInstance()->setCursor(m_pos.x + (m_pos.width/2) - (strlen(m_text) * 3 * m_textMag),
		m_pos.y + (m_pos.height/2) - (4 * m_textMag));
	}
	TFTeSPIWrapper::getInstance()->setTextColor(textColour);
	TFTeSPIWrapper::getInstance()->print(m_text);
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

} // namespace cz::gfx

