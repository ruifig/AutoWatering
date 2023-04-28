#pragma once

#include "Config/Config.h"

namespace cz
{

struct LayoutHelper
{
	static constexpr Rect getHistoryPlotRect(int index)
	{
		Rect rect(
			// Leave space to add the small markers
			AW_GROUP_NUM_WIDTH+3,
			// 1+ to leave a line for the box
			// + X at the end to add a space between different history plots
			1 + (index*(AW_GRAPH_HEIGHT + 5)),
			AW_GRAPH_NUMPOINTS, // width
			AW_GRAPH_HEIGHT // height
		);

		return rect;
	}

	static constexpr Pos getMenuButtonPos(uint8_t col, uint8_t row)
	{
		Pos pos{0};
		pos.x = (32 + 3) * col;
		pos.y = getHistoryPlotRect(3).y + getHistoryPlotRect(3).height + 3 + (32 + 3) * row;
		return pos;
	}

	static constexpr Rect getMenuButtonRect(uint8_t col, uint8_t row)
	{
		return Rect(getMenuButtonPos(col, row), 32, 32);
	}

	static constexpr Pos getMenuTopLeft()
	{
		return getMenuButtonPos(0,0);
	}

	static constexpr Pos getMenuBottomRight()
	{
		return getMenuButtonPos(8,1) + Pos{32, 32};
	}

	static constexpr Rect getMenuFullArea()
	{
		Rect rect{getMenuTopLeft(), getMenuBottomRight()};
		return rect;
	}

	static constexpr Rect getMenuLineArea(int index)
	{
		Rect rect{ getMenuButtonPos(0, index), getMenuButtonPos(8,index) + Pos{32, 32}};
		return rect;
	}

	#define STATUS_BAR_DIVISIONS 20
	static constexpr Rect getStatusBarPos()
	{
		Rect rect { getMenuButtonPos(0,2), {AW_SCREEN_WIDTH, AW_SCREEN_HEIGHT} };
		return rect;
	}

	static constexpr Rect getStatusBarCells(int startCell, int numCells)
	{
		Rect statusBar = getStatusBarPos();
		return Rect(
			statusBar.x + startCell * (statusBar.width / STATUS_BAR_DIVISIONS), statusBar.y,
			numCells * (statusBar.width / STATUS_BAR_DIVISIONS), statusBar.height
		);
	}
};
	
// Make sure we have enough space at the bottom for the menu
static_assert((AW_SCREEN_HEIGHT - (LayoutHelper::getHistoryPlotRect(3).y + LayoutHelper::getHistoryPlotRect(3).height)) > 64, "Need enough space left at the bottom for the menu (64 pixels high)");

} // namespace cz
