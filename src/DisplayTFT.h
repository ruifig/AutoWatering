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


class Group
{
  public:
  private:
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

	static const char* const ms_stateNames[3];

	Context& m_ctx;
	MCUFRIEND_kbv m_tft;
	TouchScreen m_ts;
	State m_state = State::Initializing;
	float m_timeInState = 0;
	float m_timeSinceLastTouch = 0;
	bool m_screenOff = false;

	constexpr static int16_t m_historyX = 10;
	constexpr static int16_t m_groupsStartY = 10;
	constexpr static int16_t m_spaceBetweenGroups = 20;

	void changeToState(State newState);
	void onLeaveState();
	void onEnterState();

	void drawOverview();
	void drawHistoryBoxes();

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

	template<typename T>
	void printAlignedImpl(const Box& area, HAlign halign, VAlign valign, const T* txt);

	void printAligned(const Box& area, HAlign halign, VAlign valign, const char* txt)
	{
		printAlignedImpl(area, halign, valign, txt);
	}
	void printAligned(const Box& area, HAlign halign, VAlign valign, const __FlashStringHelper* txt)
	{
		printAlignedImpl(area, halign, valign, txt);
	}

	/**
	 * Draws a filled rectangle with the specified colour, followed by a 565 RGB bitmap from PROGMEM using a bitmask
	 * (set bits = opaque, unset bits = clear).
	 **/
	void drawRGBBitmap(int16_t x, int16_t y, const uint16_t *bitmap, const uint8_t* mask, int16_t w, int16_t h, uint16_t bkgColor);

	/**
	 * Plots a dot graph starting at x,y, with height h.
	 * One graph point per value (along the x axis), and the value range (vertical) is mapped so that:
	 * - A value of 0 is drawn (y+h)
	 * - A value of 100 is drawn at (y)
	 *
	 * @param x,y Top left corner
	 * @param h Height to map the values (a maximum value takes makes the graph h in height)
	 * @param data/count data to plot
	 * @param valThreshold If a value <= this, then it uses colour GRAPH_MOISTURE_LOW_COLOUR, otherwise GRAPH_MOISTURE_OK_COLOUR
	 * @param oldData/oldCount if specified, the data being plotted is compared against this, so it draws as few pixels as possible
	 *    If this is not specified, then the function erases a full Y range per point before drawing the point.
	 **/
	void plotHistory(int16_t x, int16_t y, int16_t h, const TFixedCapacityQueue<GraphPoint>& data, uint8_t valThreshold /*, const GraphPoint* oldData = nullptr, int oldCount=0*/);

};
	
	
} // namespace cz




