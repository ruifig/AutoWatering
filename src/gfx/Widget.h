#pragma once

#include "GFXUtils.h"
#include "crazygaze/micromuc/EnumFlags.h"

namespace cz::gfx
{

enum class WidgetFlag : uint16_t
{
	None = 0,
	// If set, it fills the drawing area with the background colour
	EraseBkg = (1 << 0),
	// If set, it draw a rectangle with the foreground/text colour
	DrawBorder = (1 << 1),
	// For numeric labels, it display the text as a percentage number. E.g: "90%"
	NumAsPercentage = (1 << 2)
};
ENUM_CLASS_FLAGS(WidgetFlag);

class Widget
{
  public:
	virtual ~Widget() {}
	virtual void draw(bool forceDraw = false) = 0;
  private:
};

	
} // namespace cz::gfx

