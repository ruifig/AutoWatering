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
	m_redrawOuterBox = true; 
}

bool GroupGraph::contains(const Pos& pos) const
{
	return LayoutHelper::getHistoryPlotRect(m_index).contains(pos);
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
					m_redrawOuterBox = true;
				}
			}
			else
			{
				if (m_selected)
				{
					m_selected = false;
					m_redrawOuterBox = true;
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

	if (m_forceRedraw || m_redrawOuterBox)
	{
		drawOuterBox();
	}

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
			printAligned(LayoutHelper::getHistoryPlotRect(m_index), HAlign::Center, VAlign::Center, F("Not Running"));
		}
	}

	m_forceRedraw = false;	
}

void GroupGraph::plotHistory()
{
	PROFILE_SCOPE(F("GroupGraph::plotHistory"));

	constexpr int h = GRAPH_HEIGHT;
	Rect rect = LayoutHelper::getHistoryPlotRect(m_index);
	int bottomY = rect.bottom();
	GroupData& data = gCtx.data.getGroupData(m_index);
	const HistoryQueue& history = data.getHistory();

	unsigned int thresholdPercentage = data.getThresholdValueAsPercentage();
	int16_t thresholdMarkerY = bottomY - map(thresholdPercentage, 0, 100, 0, GRAPH_POINT_MAXVAL);

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
			// 0 means it's not set yet
			if (m_previousThresholdMarkerY!=0)
			{
				gScreen.drawPixel(xx, m_previousThresholdMarkerY, GRAPH_BKG_COLOUR);
			}

			gScreen.drawPixel(xx, thresholdMarkerY, GRAPH_THRESHOLDMARKER_COLOUR);

			// Draw new moisture level
			gScreen.drawPixel(xx, bottomY - p.val, p.status==SensorReading::Status::Valid ? GRAPH_MOISTURELEVEL_COLOUR : GRAPH_MOISTURELEVEL_ERROR_COLOUR);
		}
	}

	if (data.getSensorErrorCount() !=0)
	{
		uint16_t size = rect.height/3;
		gScreen.fillRoundRect(rect.left(), rect.bottom() - size, size, size, 3, Colour_Red);
	}

	m_previousThresholdMarkerY = thresholdMarkerY;
	m_sensorUpdates = 0;
}


void GroupGraph::drawOuterBox()
{
	PROFILE_SCOPE(F("GroupGraph::drawOuterBox"));
	
	Rect historyRect = LayoutHelper::getHistoryPlotRect(m_index);
	Rect historyOuterBox = historyRect.expand(1);
	Rect groupOuterBox = {0, historyOuterBox.top(), SCREEN_WIDTH, historyOuterBox.height};

	if (m_selected)
	{
		drawRect(groupOuterBox, GRAPH_SELECTED_BORDER_COLOUR);
	}
	else
	{
		// If the group is not selected, we need to erase the selection box
		drawRect(groupOuterBox, GRAPH_BKG_COLOUR);
		// Draw the outer box around the history plot
		drawRect(historyOuterBox, GRAPH_BORDER_COLOUR);
	}

	constexpr int16_t h = GRAPH_HEIGHT;
	int bottomY = historyRect.bottom();
	gScreen.drawFastHLine(GROUP_NUM_WIDTH+0, bottomY - map(0, 0, 100, 0, h - 1),   2, GRAPH_BORDER_COLOUR);
	gScreen.drawFastHLine(GROUP_NUM_WIDTH+0, bottomY - map(20, 0, 100, 0, h - 1),  2, GRAPH_BORDER_COLOUR);
	gScreen.drawFastHLine(GROUP_NUM_WIDTH+0, bottomY - map(40, 0, 100, 0, h - 1),  2, GRAPH_BORDER_COLOUR);
	gScreen.drawFastHLine(GROUP_NUM_WIDTH+0, bottomY - map(60, 0, 100, 0, h - 1),  2, GRAPH_BORDER_COLOUR);
	gScreen.drawFastHLine(GROUP_NUM_WIDTH+0, bottomY - map(80, 0, 100, 0, h - 1),  2, GRAPH_BORDER_COLOUR);
	gScreen.drawFastHLine(GROUP_NUM_WIDTH+0, bottomY - map(100, 0, 100, 0, h - 1), 2, GRAPH_BORDER_COLOUR);

	m_redrawOuterBox = false;
}

}

