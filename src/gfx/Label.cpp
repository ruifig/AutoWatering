#include "Label.h"

namespace cz::gfx
{

#pragma region BaseLabel
//////////////////////////////////////////////////////////////////////////
// BaseLabel
//////////////////////////////////////////////////////////////////////////
void BaseLabel::drawImplHelper(const FixedLabelData& data)
{
	if (enumHasAnyFlags(data.flags,WidgetFlag::EraseBkg))
	{
		fillRect(data.pos, (uint16_t)data.bkgColour);
	}

	if (enumHasAnyFlags(data.flags, WidgetFlag::DrawBorder))
	{
		drawRect(data.pos, (uint16_t)data.textColour);
	}

	gScreen.setFont(data.font);
	gScreen.setTextColor((uint16_t)data.textColour);
}

void BaseLabel::drawImpl(const FixedLabelData& data, const char* value)
{
	drawImplHelper(data);
	printAligned(data.pos, data.halign, data.valign, value);
}

void BaseLabel::drawImpl(const StaticLabelData& data)
{
	drawImplHelper(data.fixed);
	printAligned(data.fixed.pos, data.fixed.halign, data.fixed.valign, data.value);
}
#pragma endregion

#pragma region StaticLabel
//////////////////////////////////////////////////////////////////////////
// StaticLabel
//////////////////////////////////////////////////////////////////////////
StaticLabel::StaticLabel(const StaticLabelData* data_P)
	: m_data_P(data_P)
{
}

void StaticLabel::draw(bool forceDraw)
{
	StaticLabelData data;
	memcpy_P(&data, m_data_P, sizeof(data));
	drawImpl(data);
}

#pragma endregion


//////////////////////////////////////////////////////////////////////////
// FixedNumLabel
//////////////////////////////////////////////////////////////////////////

FixedNumLabel::FixedNumLabel(const FixedLabelData* data_P, int value)
	: m_data_P(data_P)
	, m_value(value)
	, m_needsRedraw(true)
	, m_hasValue(false)
{
}

void FixedNumLabel::draw(bool forceDraw)
{
	if (!m_needsRedraw && !forceDraw)
	{
		return;
	}

	FixedLabelData data = getFixedData();
	if (m_hasValue)
	{
		const char *str;
		if (enumHasAnyFlags(data.flags, WidgetFlag::NumAsPercentage))
		{
			str = formatString(F("%3u%%"), m_value);
		}
		else
		{
			str = itoa(m_value, getTemporaryString(), 10);
		}

		drawImpl(data, str);
	}
	else
	{
		fillRect(data.pos, data.bkgColour);
	}

	m_needsRedraw = false;
}

void FixedNumLabel::clearValue()
{
	if (m_hasValue)
	{
		m_hasValue = false;
		m_needsRedraw = true;
	}
}

void FixedNumLabel::setValue(int value)
{
	if (!m_hasValue  || (value != m_value))
	{
		m_hasValue = true;
		m_value = value;
		m_needsRedraw = true;
	}
}

void FixedNumLabel::setValueAndDraw(int value, bool forceDraw)
{
	setValue(value);
	draw(forceDraw);
}

void FixedNumLabel::clearValueAndDraw(bool forceDraw)
{
	clearValue();
	draw(forceDraw);
}

cz::Rect FixedNumLabel::getRect() const
{
	return getFixedData().pos;
}

cz::gfx::FixedLabelData FixedNumLabel::getFixedData() const
{
	FixedLabelData data;
	memcpy_P(&data, m_data_P, sizeof(data));
	return data;
}

} // namespace cz::gfx

