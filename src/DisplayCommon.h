#pragma once

namespace cz
{
	
constexpr Rect getHistoryPlotRect(int index)
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

} // namespace cz
