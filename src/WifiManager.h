#pragma once

#include "Component.h"
#include <WiFi.h>

namespace cz
{

class WifiManager : public Component
{
  public:

	WifiManager();
	virtual ~WifiManager();

	bool isConnected();

	/**
	 * Disconnects from Wifi.
	 * @param reconnect If true, it will attemp a reconnect when ticking the component
	*/
	void disconnect(bool reconnect);
  private:

	// Component interface
	virtual const char* getName() const override { return "WifiManager"; }
	virtual bool initImpl() override;
	virtual float tick(float deltaSeconds) override;
	virtual void onEvent(const Event& evt) override;
	virtual bool processCommand(const Command& cmd) override;

	void checkConnection(bool systemResetOnFail);
	void printWifiStatus();

	WiFiMulti m_multi;
	bool m_reconnect = true;
};

#if AW_WIFI_ENABLED
	extern WifiManager gWifiManager;
#endif

} // namespace cz

CZ_DECLARE_LOG_CATEGORY(logWifi, Log, Verbose)