#pragma once

#include "crazygaze/micromuc/czmicromuc.h"

#if CZ_LOG_SD_ENABLED

#include "crazygaze/micromuc/Logging.h"

/**
 * https://github.com/adafruit/SD
 */
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
	 * @param dirFile directory where to create the file
	 * @param name 8.3 DOS name
	 * @param append If true, file will be opened for append. If false, it will be truncated if it exists.
	 */
	void begin(SdFile& dirFile, const char* name, bool truncate);

  private:
	virtual void logSimple(const char* str) override;
	virtual void logSimple(const __FlashStringHelper* str) override;

	SdFile m_file;
	bool m_initialized = false;
};
	
}

#endif // CZ_LOG_SD_ENABLED
