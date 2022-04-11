#pragma once

#include "Config.h"
#include "gfx/Widget.h"
#include "Events.h"

namespace cz
{

class GroupGraph : public gfx::Widget
{
public:
	GroupGraph();
	virtual ~GroupGraph();
	virtual void draw(bool forceDraw = false) override;
	void onEvent(const Event& evt);
	void init(int8_t index);
	bool contains(const Pos& pos) const;
private:

	void plotHistory();
	void drawOuterBox();

	int8_t m_index = -1;
	uint8_t m_sensorUpdates = 0;
	uint8_t m_thresholdUpdates = 0;

	bool m_forceRedraw : 1;
	bool m_selected : 1;
	bool m_redrawOuterBox : 1;
	int16_t m_previousThresholdMarkerY = 0;
};
	
} // namespace cz

