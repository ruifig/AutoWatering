#include "WifiManager.h"
#include <Arduino.h>
#include "Watchdog.h"

CZ_DEFINE_LOG_CATEGORY(logWifi);

namespace cz
{

#if AW_WIFI_ENABLED
	WifiManager gWifiManager;
#endif


WifiManager::WifiManager()
{
}

WifiManager::~WifiManager()
{
}

bool WifiManager::isConnected()
{
	return WiFi.status() == WL_CONNECTED;
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
	checkConnection(true);
	return 0.25f;
}

void WifiManager::onEvent(const Event& evt)
{
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

	#define MAX_NUM_WIFI_CONNECT_TRIES_PER_LOOP 20
	int numTries = 0;
	while(WiFi.status() != WL_CONNECTED)
	{
		if (numTries == 0)
		{
			Component::raiseEvent(WifiConnectingEvent());
		}

		numTries++;
		CZ_LOG(logWifi, Log, "Connecting to %s (Attempt %d)", WIFI_SSID, numTries);

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

		if (numTries > MAX_NUM_WIFI_CONNECT_TRIES_PER_LOOP)
		{
			Component::raiseEvent(WifiStatusEvent(false));
			// Restart for Portenta as something is very wrong
			CZ_LOG(logWifi, Error, "Can't connect to any WiFi. Resetting");
			cz::LogOutput::flush();
			delay(1000);
			rp2040.reboot();
			return;
		}
		else
		{
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

