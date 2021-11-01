#include "GroupGraph.h"
#include "DisplayCommon.h"
#include "Context.h"
#include "crazygaze/micromuc/Profiler.h"

namespace cz
{
	
GroupGraph::GroupGraph()
{
}

GroupGraph::~GroupGraph()
{
}

void GroupGraph::init(int8_t index)
{
	CZ_ASSERT(index>=0 && index<NUM_MOISTURESENSORS);
	m_index = index;
	m_forceRedraw = true;
	m_selected = false;
	m_redrawSelectedBox = false; 
}

void GroupGraph::onEvent(const Event& evt)
{
	switch(evt.type)
	{
		case Event::ConfigLoad:
			m_forceRedraw = true;
		break;

		case Event::GroupOnOff:
		{
			auto idx = static_cast<const GroupOnOffEvent&>(evt).index;
			if (idx == m_index)
			{
				m_forceRedraw = true;
			}
		}
		break;

		case Event::GroupSelected:
		{
			const GroupSelectedEvent& e = static_cast<const GroupSelectedEvent&>(evt);
			if (e.index == m_index)
			{
				if (!m_selected)
				{
					m_selected = true;
					m_redrawSelectedBox = true;
				}
			}
			else
			{
				if (m_selected)
				{
					m_selected = false;
					m_redrawSelectedBox = true;
				}
			}
		}
		break;

		case Event::SoilMoistureSensorReading:
		{
			auto idx = static_cast<const SoilMoistureSensorReadingEvent&>(evt).index;
			if (idx == m_index)
			{
				m_sensorUpdates++;
			}
		}
		break;

		default:
		break;
	}
}

void GroupGraph::draw(bool forceDraw)
{
	CZ_ASSERT(m_index!=-1); // Check if init was called

	 m_forceRedraw |= forceDraw;

	if (m_forceRedraw)
	{
		drawOuterBox();
	}

	plotHistory();
	
	//
	// We only need to process the NOT RUNNING label if we are forcing a redraw AND the group is not running
	//
	if (m_forceRedraw)
	{
		GroupData& data = gCtx.data.getGroupData(m_index);
		if (!data.isRunning())
		{
			PROFILE_SCOPE(F("notRunning"));

			gScreen.setFont(MEDIUM_FONT);
			gScreen.setTextColor(GRAPH_NOTRUNNING_TEXT_COLOUR);
			printAligned(getHistoryPlotRect(m_index), HAlign::Center, VAlign::Center, F("Not Running"));
		}
	}

	if (m_redrawSelectedBox)
	{
		Rect rect = getHistoryPlotRect(m_index);
		drawRect(rect.expand(2), m_selected ? GRAPH_SELECTED_BORDER_COLOUR : GRAPH_NOTSELECTED_BORDER_COLOUR);
		m_redrawSelectedBox = false;
	}
	
	m_forceRedraw = false;	
}

void GroupGraph::plotHistory()
{
	PROFILE_SCOPE(F("GroupGraph::plotHistory"));

	constexpr int h = GRAPH_HEIGHT;
	Rect rect = getHistoryPlotRect(m_index);
	GroupData& data = gCtx.data.getGroupData(m_index);
	const HistoryQueue& history = data.getHistory();

	int bottomY = rect.y + h - 1;

	const int count = history.size();
	
	// If there were more than 1 update, we don't have the info to scroll. We need to redraw
	bool redraw = m_sensorUpdates > 1 ? true : false;
	if (redraw)
	{
		CZ_LOG(logDefault, Warning, F("Too many sensor udpates (%u)"), (unsigned int)m_sensorUpdates);
	}
	
	redraw |= m_forceRedraw;

	for(int i=1; i<count; i++)
	{
		int xx = rect.x + i - 1;
		GraphPoint p = history.getAtIndex(i);
		GraphPoint oldp = history.getAtIndex(i-1);

		bool drawMotor = false;
		bool drawLevel = false;

		if (redraw)
		{
			// erase vertical line
			gScreen.drawFastVLine(xx, rect.y, h, GRAPH_BKG_COLOUR);
			drawMotor = true;
			drawLevel = true;
		}
		else
		{
			// Since the motor on/off info is an horizontal line, we don't need to erase the previous. We just paint the pixel
			// at the right color if it changed
			if (oldp.on != p.on)
			{
				drawMotor = true;
			}

			if (oldp.val != p.val)
			{
				// Erase previous moisture level point
				gScreen.drawPixel(xx, bottomY - oldp.val, GRAPH_BKG_COLOUR);
				drawLevel = true;
			}
		}

		if (drawMotor)
		{
			// draw new motor on/off
			gScreen.drawPixel(xx, rect.y, p.on ? GRAPH_MOTOR_ON_COLOUR : GRAPH_MOTOR_OFF_COLOUR);
		}

		if (drawLevel)
		{
			// Draw new moisture level
			gScreen.drawPixel(xx, bottomY - p.val, GRAPH_MOISTURELEVEL_COLOUR);
		}
	}

	m_sensorUpdates = 0;
}


void GroupGraph::drawOuterBox()
{
	PROFILE_SCOPE(F("GroupGraph::drawOuterBox"));
	
	Rect rect = getHistoryPlotRect(m_index);
	drawRect(rect.expand(1), GRAPH_BORDER_COLOUR);

	constexpr int16_t h = GRAPH_HEIGHT;
	int bottomY = rect.y + rect.height - 1;
	gScreen.drawFastHLine(0, bottomY - map(0, 0, 100, 0, h - 1),   2, GRAPH_BORDER_COLOUR);
	gScreen.drawFastHLine(0, bottomY - map(20, 0, 100, 0, h - 1),  2, GRAPH_BORDER_COLOUR);
	gScreen.drawFastHLine(0, bottomY - map(40, 0, 100, 0, h - 1),  2, GRAPH_BORDER_COLOUR);
	gScreen.drawFastHLine(0, bottomY - map(60, 0, 100, 0, h - 1),  2, GRAPH_BORDER_COLOUR);
	gScreen.drawFastHLine(0, bottomY - map(80, 0, 100, 0, h - 1),  2, GRAPH_BORDER_COLOUR);
	gScreen.drawFastHLine(0, bottomY - map(100, 0, 100, 0, h - 1), 2, GRAPH_BORDER_COLOUR);
}

}