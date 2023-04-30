#include "MQTTUI.h"

namespace cz
{

#if AW_MQTTUI_ENABLED
	MQTTUI gMQTTUI;
#endif

bool MQTTUI::initImpl()
{
	return true;
}

float MQTTUI::tick(float deltaSeconds)
{
	return 0.1f;
}

void MQTTUI::onEvent(const Event& evt)
{
} 

bool MQTTUI::processCommand(const Command& cmd)
{
	return false;
}

} // namespace cz
