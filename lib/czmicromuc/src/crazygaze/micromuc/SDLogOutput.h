#pragma once

#include "crazygaze/micromuc/Logging.h"
#include "SdFat.h"
#include <Arduino.h>

namespace cz
{

/**
 * Helper struct to help initialize an sdcard and volume 
 */
struct SDCardHelper
{
	Sd2Card card;
	SdVolume volume;
	SdFile root;

	/*
	 * @param SSPin What pin to use for device select
	 */
	bool begin(int SSPin);
};

class SDLogOutput : public LogOutput
{
  public:
	SDLogOutput();
	~SDLogOutput();

	/**
	 * @param SSPin What pin to use for device select
	 * @param name 8.3 DOS name
	 * @param append If true, file will be opened for append. If false, it will be truncated if it exists.
	 */
	void begin(const char* name, bool append);

  private:
	virtual void log(const LogCategoryBase* category, LogVerbosity verbosity, const char* msg) override;
	virtual void logSimple(LogVerbosity verbosity, const char* str) override;
	virtual void logSimple(LogVerbosity verbosity, const __FlashStringHelper* str) override;

	bool m_initialized = false;
};
	
}