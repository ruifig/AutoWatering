#include "GroupGraph.h"
#include "DisplayCommon.h"
#include "Context.h"
#include "crazygaze/micromuc/Profiler.h"
#include "gfx/TFTeSPIWrapper.h"

namespace cz
{

GroupGraph::GroupGraph()
{
}

GroupGraph::~GroupGraph()
{
}

void GroupGraph::setAssociation(int8_t screenSlot, int8_t pairIndex)
{
	CZ_ASSERT(pairIndex >=-1 && pairIndex < AW_MAX_NUM_PAIRS);
	CZ_ASSERT(screenSlot >= 0 && screenSlot < AW_VISIBLE_NUM_PAIRS);
	m_pairIndex = pairIndex;
	m_screenSlot = screenSlot;
	m_sensorUpdates = 0;
	m_thresholdUpdates = 0;
	m_previousThresholdMarkerY = 0;

	m_forceRedraw = true;
	m_selected = false;
	m_redrawOuterBox = true; 
}

Rect GroupGraph::getScreenSlotRect() const
{
	return LayoutHelper::getHistoryPlotRect(m_screenSlot);
}

void GroupGraph::onEvent(const Event& evt)
{
	switch(evt.type)
	{
		case Event::ConfigLoad:
		{
			int8_t group = static_cast<const ConfigLoadEvent&>(evt).group;
			if (group == m_pairIndex)
			{
				m_forceRedraw = true;
			}
		}
		break;

		case Event::ConfigSave:
		{
			int8_t group = static_cast<const ConfigSaveEvent&>(evt).group;
			if (group == m_pairIndex)
			{
				// If the config was saved, probably something changed, so we redraw everything
				m_forceRedraw = true;
			}
		}
		break;

		case Event::GroupOnOff:
		{
			auto idx = static_cast<const GroupOnOffEvent&>(evt).index;
			if (idx == m_pairIndex)
			{
				m_forceRedraw = true;
			}
		}
		break;

		case Event::GroupSelected:
		{
			const GroupSelectedEvent& e = static_cast<const GroupSelectedEvent&>(evt);

			if (m_pairIndex==-1)
			{
				// No pair associated with this object, so do nothing
			}
			else
			{
				if (e.index == m_pairIndex)
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
		}
		break;

		case Event::SoilMoistureSensorReading:
		{
			auto idx = static_cast<const SoilMoistureSensorReadingEvent&>(evt).index;
			if (idx == m_pairIndex)
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
	CZ_ASSERT( m_screenSlot != -1); // Check if init was called

	// If not associated with a pair, do nothing.
	if (m_pairIndex==-1)
	{
		return;
	}

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
		GroupData& data = gCtx.data.getGroupData(m_pairIndex);
		if (!data.isRunning())
		{
			PROFILE_SCOPE(F("notRunning"));

			TFTeSPIWrapper::getInstance()->setFont(MEDIUM_FONT);
			TFTeSPIWrapper::getInstance()->setTextColor(AW_GRAPH_NOTRUNNING_TEXT_COLOUR);
			printAligned(getScreenSlotRect(), HAlign::Center, VAlign::Center, F("Not Running"));
		}
	}

	m_forceRedraw = false;	
}

void GroupGraph::plotHistory()
{
	PROFILE_SCOPE(F("GroupGraph::plotHistory"));

	constexpr int h = AW_GRAPH_HEIGHT;
	Rect rect = getScreenSlotRect();
	int bottomY = rect.bottom();
	GroupData& data = gCtx.data.getGroupData(m_pairIndex);
	const HistoryQueue& history = data.getHistory();

	unsigned int thresholdPercentage = data.getThresholdValueAsPercentage();
	int16_t thresholdMarkerY = bottomY - map(thresholdPercentage, 0, 100, 0, AW_GRAPH_POINT_MAXVAL);

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
			TFTeSPIWrapper::getInstance()->drawFastVLine(xx, rect.y, h, AW_GRAPH_BKG_COLOUR);
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
				TFTeSPIWrapper::getInstance()->drawPixel(xx, bottomY - oldp.val, AW_GRAPH_BKG_COLOUR);
				drawLevel = true;
			}
		}

		if (drawMotor)
		{
			// draw new motor on/off
			TFTeSPIWrapper::getInstance()->drawPixel(xx, rect.y, p.motorOn ? AW_GRAPH_MOTOR_ON_COLOUR : AW_GRAPH_MOTOR_OFF_COLOUR);
		}

		if (drawLevel)
		{
			// 0 means it's not set yet
			if (m_previousThresholdMarkerY != 0)
			{
				TFTeSPIWrapper::getInstance()->drawPixel(xx, m_previousThresholdMarkerY, AW_GRAPH_BKG_COLOUR);
			}

			TFTeSPIWrapper::getInstance()->drawPixel(xx, thresholdMarkerY, AW_GRAPH_THRESHOLDMARKER_COLOUR);

			// Draw new moisture level
			TFTeSPIWrapper::getInstance()->drawPixel(xx, bottomY - p.val, p.status==SensorReading::Status::Valid ? AW_GRAPH_MOISTURELEVEL_COLOUR : AW_GRAPH_MOISTURELEVEL_ERROR_COLOUR);
		}
	}

	if (data.getSensorErrorCount() !=0)
	{
		uint16_t size = rect.height/3;
		TFTeSPIWrapper::getInstance()->fillRoundRect(rect.left(), rect.bottom() - size, size, size, 3, Colour_Red);
	}

	m_previousThresholdMarkerY = thresholdMarkerY;
	m_sensorUpdates = 0;
}


void GroupGraph::drawOuterBox()
{
	PROFILE_SCOPE(F("GroupGraph::drawOuterBox"));
	
	Rect historyRect = getScreenSlotRect();
	Rect historyOuterBox = historyRect.expand(1);
	Rect groupOuterBox = {0, historyOuterBox.top(), AW_SCREEN_WIDTH, historyOuterBox.height};

	if (m_selected)
	{
		drawRect(groupOuterBox, AW_GRAPH_SELECTED_BORDER_COLOUR);
	}
	else
	{
		// If the group is not selected, we need to erase the selection box
		drawRect(groupOuterBox, AW_GRAPH_BKG_COLOUR);
		// Draw the outer box around the history plot
		drawRect(historyOuterBox, AW_GRAPH_BORDER_COLOUR);
	}

	constexpr int16_t h = AW_GRAPH_HEIGHT;
	int bottomY = historyRect.bottom();
	TFTeSPIWrapper::getInstance()->drawFastHLine(AW_GROUP_NUM_WIDTH+0, bottomY - map(0, 0, 100, 0, h - 1),   2, AW_GRAPH_BORDER_COLOUR);
	TFTeSPIWrapper::getInstance()->drawFastHLine(AW_GROUP_NUM_WIDTH+0, bottomY - map(20, 0, 100, 0, h - 1),  2, AW_GRAPH_BORDER_COLOUR);
	TFTeSPIWrapper::getInstance()->drawFastHLine(AW_GROUP_NUM_WIDTH+0, bottomY - map(40, 0, 100, 0, h - 1),  2, AW_GRAPH_BORDER_COLOUR);
	TFTeSPIWrapper::getInstance()->drawFastHLine(AW_GROUP_NUM_WIDTH+0, bottomY - map(60, 0, 100, 0, h - 1),  2, AW_GRAPH_BORDER_COLOUR);
	TFTeSPIWrapper::getInstance()->drawFastHLine(AW_GROUP_NUM_WIDTH+0, bottomY - map(80, 0, 100, 0, h - 1),  2, AW_GRAPH_BORDER_COLOUR);
	TFTeSPIWrapper::getInstance()->drawFastHLine(AW_GROUP_NUM_WIDTH+0, bottomY - map(100, 0, 100, 0, h - 1), 2, AW_GRAPH_BORDER_COLOUR);

	m_redrawOuterBox = false;
}

bool GroupGraph::processTouch(const Pos& pos)
{
	if (m_pairIndex!=-1 && getScreenSlotRect().contains(pos))
	{
		gCtx.data.trySetSelectedGroup(m_pairIndex);
		return true;
	}
	else
	{
		return false;
	}
}

} // namespace cz

