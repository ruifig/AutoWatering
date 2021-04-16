#pragma once
#include "Context.h"
#include <Adafruit_GFX.h>
#include <MCUFRIEND_kbv.h>
#include <TouchScreen.h>

namespace cz
{

struct Box
{
	int16_t x;
	int16_t y;
	uint16_t width;
	uint16_t height;
};

class MyButton : public Adafruit_GFX_Button
{
public:

	void drawButton(boolean inverted = false ) {
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
	
};

class DisplayTFT
{
  public:
	DisplayTFT(Context& ctx);
	
	// Disable copying
	DisplayTFT(const DisplayTFT&) = delete;
	DisplayTFT& operator=(const DisplayTFT&) = delete;

	void begin();
	float tick(float deltaSeconds);

  private:

	enum class State : uint8_t
	{
		Initializing,
		Intro,
		Overview
	};

#if LOG_ENABLED
	static const char* ms_stateNames[3];
#endif

	Context& m_ctx;
	MCUFRIEND_kbv m_tft;
	TouchScreen m_ts;
	State m_state = State::Initializing;
	float m_timeInState = 0;
	float m_timeSinceLastTouch = 0;
	bool m_screenOff = false;

	void changeToState(State newState);
	void onLeaveState();
	void onEnterState();

	enum class HAlign : uint8_t
	{
		Left,
		Center,
		Right
	};

	enum class VAlign : uint8_t
	{
		Top,
		Center,
		Bottom
	};

	void printAligned(const Box& area, HAlign halign, VAlign valign, const char* txt);

};
	
	
} // namespace cz




