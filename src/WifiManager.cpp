#include "WifiManager.h"
#include <Arduino.h>
#include "Watchdog.h"
#include <crazygaze/micromuc/Profiler.h>

CZ_DEFINE_LOG_CATEGORY(logWifi);

namespace cz
{

#if AW_WIFI_ENABLED
	WifiManager gWifiManager;
#endif


WifiManager* WifiManager::ms_instance;

WifiManager::WifiManager()
{
	CZ_ASSERT(ms_instance == nullptr);
	ms_instance = this;
	// We only start ticking when we ready a ConfigReady event
	stopTicking();
}

WifiManager::~WifiManager()
{
	ms_instance = nullptr;
}

WifiManager* WifiManager::getInstance()
{
	return ms_instance;
}

bool WifiManager::isConnected() const
{
	return WiFi.status() == WL_CONNECTED;
}

bool WifiManager::willReconnect() const
{
	return m_reconnect;
}

void WifiManager::disconnect(bool reconnect)
{
	m_reconnect = reconnect;
	WiFi.disconnect();
	Component::raiseEvent(WifiStatusEvent(false));
}

bool WifiManager::initImpl()
{
	m_multi.addAP(WIFI_SSID, WIFI_PASSWORD);
	return true;
}

float WifiManager::tick(float deltaSeconds)
{
	PROFILE_SCOPE(F("WifiManager"));

	checkConnection(true);
	return 0.25f;
}

void WifiManager::onEvent(const Event& evt)
{
	switch(evt.type)
	{
		case Event::Type::ConfigReady:
		{
			startTicking();
		}
		break;
	}
}

bool WifiManager::processCommand(const Command& cmd)
{
	if (cmd.is("disconnect"))
	{
		disconnect(true);
	}
	else
	{
		return false;
	}
	
	return true;
}

void WifiManager::checkConnection(bool systemResetOnFail)
{
	if (WiFi.status() == WL_CONNECTED)
	{
		return;
	}

	if (!m_reconnect)
	{
		return;
	}

	int numTries = 0;
	while(WiFi.status() != WL_CONNECTED)
	{
		if (numTries == 0)
		{
			Component::raiseEvent(WifiConnectingEvent());
		}

		numTries++;
		CZ_LOG(logWifi, Log, "Connecting to %s (Attempt %d out of %d)", WIFI_SSID, numTries, AW_WIFI_CONNECT_NUM_TRIES);

		// Disable the watchdog ONLY for as long as we need
		uint8_t runRes;
		{
			WatchdogPauseScope wtdPause;
			runRes = m_multi.run();
		}

		if (runRes == WL_CONNECTED)
		{
			//CZ_LOG(logWifi, Log, "Wifi connected. IP %s", m_wifiClient.localIP().toString().c_str());
			printWifiStatus();
			break;
		}

		if (numTries >= AW_WIFI_CONNECT_NUM_TRIES)
		{
			Component::raiseEvent(WifiStatusEvent(false));
			if constexpr (AW_WIFI_REBOOT_CONNECT_FAILURE == 0)
			{
				CZ_LOG(logWifi, Error, "Can't connect to any WiFi.");
				m_reconnect = false;
			}
			else
			{
				// Restart for Portenta as something is very wrong
				CZ_LOG(logWifi, Error, "Can't connect to any WiFi. Resetting");
				cz::LogOutput::flush();
				delay(1000);
				rp2040.reboot();
			}
			return;
		}
		else
		{
			CZ_LOG(logWifi, Error, "Can't connect to any WiFi. Retrying.");
			delay(200);
		}
	}

	Component::raiseEvent(WifiStatusEvent(true));
}

void WifiManager::printWifiStatus()
{
	// print the SSID of the network you're attached to:
	CZ_LOG(logWifi, Log, "Connected to SSID: %s", WiFi.SSID().c_str());

	// print your board's IP address:
	IPAddress ip = WiFi.localIP();
	CZ_LOG(logWifi, Log, "Local IP Address: %s", ip.toString().c_str());

	// print the received signal strength:
	int rssi = WiFi.RSSI();
	CZ_LOG(logWifi, Log, "Signal strenght (RSSI): %d dBm", rssi)
}

} // namespace cz

