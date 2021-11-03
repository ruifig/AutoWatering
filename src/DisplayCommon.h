#pragma once

namespace cz
{
	
constexpr Rect getHistoryPlotRect(int index)
{
	Rect rect(
		// Don't start a 0, so we leave space to add the small markers
		3,
		// 1+ to leave a line for the box
		// + X at the end to add a space between different history plots
		1 + (index*(GRAPH_HEIGHT + 5)),
		GRAPH_NUMPOINTS, // width
		GRAPH_HEIGHT // height
	);
	return rect;
}

} // namespace cz