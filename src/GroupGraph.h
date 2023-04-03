#pragma once

#include "Config/Config.h"
#include "gfx/Widget.h"
#include "Events.h"

namespace cz
{

class GroupGraph : public gfx::Widget
{
public:
	GroupGraph();
	virtual ~GroupGraph();

	/**
	 * Sets what sensor/motor pair and screen slot this graph is associated with
	 * 
	 * \param screenSlot What screen slot use (where to draw)
	 * \param pairIndex
	 * 		If [0..MAX_NUM_PAIRS[ it associates with that sensor/motor pair
	 * 		If -1, then it means no association	
	 */
	void setAssociation(int8_t screenSlot, int8_t pairIndex);

	/**
	 * Sets this group graph as not having any pair association.
	 */
	void clearAssociation();

	virtual void draw(bool forceDraw = false) override;
	void onEvent(const Event& evt);

	bool processTouch(const Pos& pos);

private:

	Rect getScreenSlotRect() const;
	void plotHistory();
	void drawOuterBox();

	// What sensor/motor group this is associated
	int8_t m_pairIndex = -1;

	// Where int the screen to display it.
	// The screen is arranged to accomodate up to VISIBLE_NUM_PAIRS. This tells in what slot this group
	// is on
	int8_t m_screenSlot = -1;

	uint8_t m_sensorUpdates = 0;
	uint8_t m_thresholdUpdates = 0;
	int16_t m_previousThresholdMarkerY = 0;

	bool m_forceRedraw : 1;
	bool m_selected : 1;
	bool m_redrawOuterBox : 1;
};
	
} // namespace cz

