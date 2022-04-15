#pragma once

#include "Config.h"

namespace cz
{

struct LayoutHelper
{
	static constexpr Rect getHistoryPlotRect(int index)
	{
		Rect rect(
			// Leave space to add the small markers
			GROUP_NUM_WIDTH+3,
			// 1+ to leave a line for the box
			// + X at the end to add a space between different history plots
			1 + (index*(GRAPH_HEIGHT + 5)),
			GRAPH_NUMPOINTS, // width
			GRAPH_HEIGHT // height
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

	#define STATUS_BAR_DIVISIONS 20
	static constexpr Rect getStatusBarPos()
	{
		Rect rect { getMenuButtonPos(0,2), {SCREEN_WIDTH, SCREEN_HEIGHT} };
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
static_assert((SCREEN_HEIGHT - (LayoutHelper::getHistoryPlotRect(3).y + LayoutHelper::getHistoryPlotRect(3).height)) > 64, "Need enough space left at the bottom for the menu (64 pixels high)");

} // namespace cz
