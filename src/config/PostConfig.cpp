
#include <Arduino.h>
#include <crazygaze/micromuc/Logging.h>

#if AW_WIFI_ENABLED
	static_assert(AW_WIFI_CONNECT_NUM_TRIES >= 1, "Invalid value for AW_WIFI_CONNECT_NUM_TRIES");
#endif

namespace cz
{

void Setup::createSoilMoistureSensors()
{
	CZ_LOG(logDefault, Log, "Creating soil moisture sensor components");
	for(int i=0; i<AW_MAX_NUM_PAIRS; i++)
	{
		m_soilMoistureSensors[i] = gSetup->createSoilMoistureSensor(i);
	}
}

void Setup::createPumpMonitors()
{
	CZ_LOG(logDefault, Log, "Creating group monitor components");
	for(int i=0; i<AW_MAX_NUM_PAIRS; i++)
	{
		m_pumpMonitors[i] = gSetup->createPumpMonitor(i);
	}
}

} // namespace cz
