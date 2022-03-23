#if PORTING_TO_RP2040

#include "Label.h"

namespace cz::gfx
{

#pragma region BaseLabel
//////////////////////////////////////////////////////////////////////////
// BaseLabel
//////////////////////////////////////////////////////////////////////////
void BaseLabel::drawImplHelper(const LabelData& data)
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

void BaseLabel::drawImpl(const LabelData& data, const char* value)
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

} // namespace cz::gfx

#endif
