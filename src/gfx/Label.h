#pragma once

#include "Widget.h"

namespace cz::gfx
{

/**
 * Data for a label that the text can change, but not the other parameters
 */
struct FixedLabelData
{
	Rect pos;
	HAlign halign;
	VAlign valign;
	const GFXfont* font;
	Colour textColour;
	Colour bkgColour;
	WidgetFlag flags;
};

/**
 * Data for a label that doesn't change at all, and all data can be in PROGMEM
 */
struct StaticLabelData
{
	FixedLabelData fixed;
	const __FlashStringHelper* value;
};


class BaseLabel : public Widget
{

protected:
	void drawImplHelper(const FixedLabelData& data);
	void drawImpl(const FixedLabelData& data, const char* value);
	void drawImpl(const StaticLabelData& data);
};

/**
 * Label where nothing can change.
 * The entire setup can be put in PROGMEM
 */
class StaticLabel : public BaseLabel
{
  public:
	StaticLabel(const StaticLabelData* data_P);
	virtual void draw(bool forceDraw = false) override;
  private:
	const StaticLabelData* m_data_P;
};

/**
 * Label that the text can change, but not the rest of the setup.
 */
template<int BUFSIZE=10>
class FixedLabel : public BaseLabel
{
  public:
	static int constexpr m_bufSize = BUFSIZE;

	FixedLabel(const FixedLabelData* data_P, const __FlashStringHelper* value = nullptr)
		: m_data_P(data_P)
	{
		if (value)
		{
			strncpy_P(m_value, (const char*)value, m_bufSize);
			m_value[m_bufSize-1] = 0;
		}
		else
		{
			strncpy_P(m_value, (const char*)F("99%"), m_bufSize);
			m_value[m_bufSize-1] = 0;
		}
	}

	virtual void draw(bool forceDraw = false) override
	{
		FixedLabelData data;
		memcpy_P(&data, m_data_P, sizeof(data));
		drawImpl(data, m_value);
	}

  private:
	const FixedLabelData* m_data_P;
	char m_value[m_bufSize];
};

/**
 * Label that displays a number
 */
class FixedNumLabel : public BaseLabel
{
  public:

	FixedNumLabel(const FixedLabelData* data_P, int value = 0);
	virtual void draw(bool forceDraw = false) override;
	void setValue(int value);
	void setValueAndDraw(int value, bool forceDraw = false);

  private:
	const FixedLabelData* m_data_P;
	int m_value;
	bool m_needsRedraw;
};

	
} // namespace cz::gfx

