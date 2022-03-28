#include "Label.h"
#include "MyDisplay1.h"

namespace cz::gfx
{

extern MyDisplay1 gScreen;

//////////////////////////////////////////////////////////////////////////
// BaseLabel
//////////////////////////////////////////////////////////////////////////
void BaseLabel::drawImplHelper(const LabelData& data)
{
	if (enumHasAnyFlags(data.flags,WidgetFlag::EraseBkg))
	{
		fillRect(data.pos, data.bkgColour);
	}

	if (enumHasAnyFlags(data.flags, WidgetFlag::DrawBorder))
	{
		drawRect(data.pos, data.textColour);
	}

	gScreen.setFont(data.font);
	gScreen.setTextColor(data.textColour);
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

} // namespace cz::gfx
