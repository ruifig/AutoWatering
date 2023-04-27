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

  private:

	// Component interface
	virtual const char* getName() const override { return "WifiManager"; }
	virtual bool initImpl() override;
	virtual float tick(float deltaSeconds) override;
	virtual void onEvent(const Event& evt) override;
	virtual bool processCommand(const Command& cmd) override;

	bool connect(bool systemResetOnFail);
	void printWifiStatus();

	WiFiMulti m_multi;

};

#if WIFI_ENABLED
	extern WifiManager gWifiManager;
#endif

} // namespace cz

CZ_DECLARE_LOG_CATEGORY(logWifi, Log, Verbose)
