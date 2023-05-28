#include "Label.h"
#include "TFTeSPIWrapper.h"

namespace cz::gfx
{

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

	TFTeSPIWrapper::getInstance()->setFont(data.font);
	TFTeSPIWrapper::getInstance()->setTextColor(data.textColour);
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
