#include "GroupGraph.h"
#include "DisplayCommon.h"
#include "Context.h"
#include "crazygaze/micromuc/Profiler.h"
#include "gfx/MyDisplay1.h"

namespace cz
{

extern MyDisplay1 gScreen;
	
GroupGraph::GroupGraph()
{
}

GroupGraph::~GroupGraph()
{
}

void GroupGraph::init(int8_t index)
{
	CZ_ASSERT(index>=0 && index<NUM_PAIRS);
	m_index = index;
	m_forceRedraw = true;
	m_selected = false;
	m_redrawSelectedBox = false; 
}

bool GroupGraph::contains(const Pos& pos) const
{
	return getHistoryPlotRect(m_index).contains(pos);
}

void GroupGraph::onEvent(const Event& evt)
{
	switch(evt.type)
	{
		case Event::ConfigLoad:
		{
			int8_t group = static_cast<const ConfigLoadEvent&>(evt).group;
			if (group == m_index)
			{
				m_forceRedraw = true;
			}
		}
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
		//drawThresholdMarker();
	}

	// #TODO Remove this and draw only when necessary
	drawThresholdMarker();

	if (m_forceRedraw || m_sensorUpdates)
	{
		plotHistory();
	}
	
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
			if (oldp.motorOn != p.motorOn)
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
			gScreen.drawPixel(xx, rect.y, p.motorOn ? GRAPH_MOTOR_ON_COLOUR : GRAPH_MOTOR_OFF_COLOUR);
		}

		if (drawLevel)
		{
			// Draw new moisture level
			gScreen.drawPixel(xx, bottomY - p.val, p.status==SensorReading::Status::Valid ? GRAPH_MOISTURELEVEL_COLOUR : GRAPH_MOISTURELEVEL_ERROR_COLOUR);
		}
	}

	if (data.getSensorErrorCount() !=0)
	{
		uint16_t size = rect.height/3;
		gScreen.fillRoundRect(rect.left(), rect.bottom() - size, size, size, 3, Colour_Red);
	}

	m_sensorUpdates = 0;
}


void GroupGraph::drawOuterBox()
{
	PROFILE_SCOPE(F("GroupGraph::drawOuterBox"));
	
	Rect rect = getHistoryPlotRect(m_index);
	drawRect(rect.expand(1), GRAPH_BORDER_COLOUR);

	constexpr int16_t h = GRAPH_HEIGHT;
	int bottomY = rect.bottom();
	gScreen.drawFastHLine(0, bottomY - map(0, 0, 100, 0, h - 1),   2, GRAPH_BORDER_COLOUR);
	gScreen.drawFastHLine(0, bottomY - map(20, 0, 100, 0, h - 1),  2, GRAPH_BORDER_COLOUR);
	gScreen.drawFastHLine(0, bottomY - map(40, 0, 100, 0, h - 1),  2, GRAPH_BORDER_COLOUR);
	gScreen.drawFastHLine(0, bottomY - map(60, 0, 100, 0, h - 1),  2, GRAPH_BORDER_COLOUR);
	gScreen.drawFastHLine(0, bottomY - map(80, 0, 100, 0, h - 1),  2, GRAPH_BORDER_COLOUR);
	gScreen.drawFastHLine(0, bottomY - map(100, 0, 100, 0, h - 1), 2, GRAPH_BORDER_COLOUR);

}

void GroupGraph::drawThresholdMarker()
{
	PROFILE_SCOPE(F("GroupGraph::drawThresholdMarker"));
	
	VLine line = getHistoryPlotRect(m_index).rightLine();
	// getHistoryPlotRect gives us the inner area, where we draw the history, but we want to draw the marker
	// on the border line
	line.p.x++;
	gScreen.drawVLine(line, Colour_VeryDarkGrey);

	// Draw a pixel for the threshold level on the right side of the graph box
	// This allows the user to have an idea of the threshold value without having to go to the group settings
	GroupData& data = gCtx.data.getGroupData(m_index);
	unsigned int percentageThreshold = data.getPercentageThreshold();
	//CZ_LOG(logDefault, Log, F("Val=%u"), percentageThreshold);
#if 0
	gScreen.drawPixel(rect.x + rect.width -1, bottomY - map(percentageThreshold, 0, 100, 0, rect.height), GRAPH_MOTOR_ON_COLOUR);
#endif
	gScreen.drawPixel(line.p.x, line.bottom() - map(percentageThreshold, 0, 100, 0, GRAPH_POINT_MAXVAL), GRAPH_MOTOR_ON_COLOUR);

#if 0
	CZ_LOG(logDefault, Log, F("Markers=%d, %d, %d"),
		map(percentageThreshold, 0, 100, 0, line.height),
		map(0, 0, 100, 0, line.height),
		map(100, 0, 100, 0, line.height)
		);

	gScreen.drawPixel(line.p.x, line.bottom() - map(0, 0, 100, 0, GRAPH_POINT_MAXVAL) -1, Colour_Pink);
	gScreen.drawPixel(line.p.x, line.bottom() - map(100, 0, 100, 0, GRAPH_POINT_MAXVAL), Colour_Yellow);
	gScreen.drawPixel(line.p.x, line.bottom() - map(50, 0, 100, 0, GRAPH_POINT_MAXVAL), Colour_Green);
#endif
}

}

