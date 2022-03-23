#include "SDLogOutput.h"

#if CZ_LOG_SD_ENABLED

namespace cz
{

bool SDCardHelper::begin(int SSPin)
{
	CZ_LOG(logDefault, Log, F("Initializing SD card with CS/SS pin %d..."), SSPin);

	// we'll use the initialization code from the utility libraries
	// since we're just testing if the card is working!
	if (!card.init(SPI_FULL_SPEED, SSPin))
	{
		CZ_LOG(logDefault, Error, F("initialization failed. Things to check:"));
		CZ_LOG(logDefault, Error, F("* is a card inserted?"));
		CZ_LOG(logDefault, Error, F("* is your wiring correct?"));
		CZ_LOG(logDefault, Error, F("* did you change the chipSelect pin to match your shield or module?"));
		return false;
	}

	CZ_LOG(logDefault, Log, F("Wiring is correct and a card is present."));

	const char* cardType = "";
	switch (card.type())
	{
	case SD_CARD_TYPE_SD1:
		cardType = "SD1";
	break;
	case SD_CARD_TYPE_SD2:
		cardType = "SD2";
	break;
	case SD_CARD_TYPE_SDHC:
		cardType = "SDHC";
	break;
	default:
		cardType = "Unknown";
	}
	CZ_LOG(logDefault, Log, F("Card type: %s"), cardType);

	// Now we will try to open the 'volume'/'partition' - it should be FAT16 or FAT32
	if (!volume.init(card))
	{
		CZ_LOG(logDefault, Error, F("Could not find FAT16/FAT32 partition. Make sure you've formatted the card"));
		return false;
	}

	CZ_LOG(logDefault, Log, F("Clusters: %u"), volume.clusterCount());
	CZ_LOG(logDefault, Log, F("Blocks x Cluster: %u "), (uint32_t)volume.blocksPerCluster());
	CZ_LOG(logDefault, Log, F("Total Blocks: %u"), volume.blocksPerCluster() * volume.clusterCount());

	// print the type and size of the first FAT-type volume
	CZ_LOG(logDefault, Log, F("Volume type is: FAT%d"), volume.fatType());

	uint32_t volumesize;
	volumesize = volume.blocksPerCluster();    // clusters are collections of blocks
	volumesize *= volume.clusterCount();       // we'll have a lot of clusters
	volumesize /= 2;                           // SD card blocks are always 512 bytes (2 blocks are 1KB)
	CZ_LOG(logDefault, Log, F("Volume size (Kb): %u"), volumesize);
	volumesize /= 1024;
	CZ_LOG(logDefault, Log, F("Volume size (Mb): %u"), volumesize);

	if (root.openRoot(volume) == false)
	{
		CZ_LOG(logDefault, Error, F("Error opening sd root"));
		return false;
	}

	// list all files in the card with date and size
	// CZ_LOG(logDefault, Log, F("Files found on the card (name, date and size in bytes): "));
	// root.ls(LS_R | LS_DATE | LS_SIZE);

	return true;
}
	
SDLogOutput::SDLogOutput()
{
}

SDLogOutput::~SDLogOutput()
{
}

void SDLogOutput::begin(SdFile& dirFile, const char* name, bool truncate)
{
	int a = O_CREAT | O_WRITE;
	int flags = (O_CREAT | O_WRITE | O_APPEND) | (truncate ? O_TRUNC : 0);
	int b = flags & (O_CREAT | O_WRITE);

	if (!m_file.open(dirFile, name, (O_CREAT | O_WRITE | O_APPEND) | (truncate ? O_TRUNC : 0)))
	{
		CZ_LOG(logDefault, Error, F("Error opening log file %s"), name);
		return;
	}
	
	m_initialized = true;
}

void SDLogOutput::logSimple(const char* str)
{
	if (!m_initialized)
	{
		return;
	}

	m_file.write(str);
	m_file.sync(false);
}

void SDLogOutput::logSimple(const __FlashStringHelper* str)
{
	if (!m_initialized)
	{
		return;
	}

	m_file.write_P((const char*)str);
	m_file.sync(false);
}

	
} // namespace cz

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// Arduino SD card test example
#if 0
/*
  SD card test

  This example shows how use the utility libraries on which the'
  SD library is based in order to get info about your SD card.
  Very useful for testing a card when you're not sure whether its working or not.

  The circuit:
    SD card attached to SPI bus as follows:
 ** MOSI - pin 11 on Arduino Uno/Duemilanove/Diecimila
 ** MISO - pin 12 on Arduino Uno/Duemilanove/Diecimila
 ** CLK - pin 13 on Arduino Uno/Duemilanove/Diecimila
 ** CS - depends on your SD card shield or module.
 		Pin 4 used here for consistency with other Arduino examples


  created  28 Mar 2011
  by Limor Fried
  modified 9 Apr 2012
  by Tom Igoe
*/
// include the SD library:
#include <SPI.h>
#include <SD.h>

// set up variables using the SD utility library functions:
Sd2Card card;
SdVolume volume;
SdFile root;

// change this to match your SD shield or module;
// Arduino Ethernet shield: pin 4
// Adafruit SD shields and modules: pin 10
// Sparkfun SD shield: pin 8
// MKRZero SD: SDCARD_SS_PIN
const int chipSelect = 53;

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }


  Serial.print("\nInitializing SD card...");

  // we'll use the initialization code from the utility libraries
  // since we're just testing if the card is working!
  if (!card.init(SPI_HALF_SPEED, chipSelect)) {
    Serial.println("initialization failed. Things to check:");
    Serial.println("* is a card inserted?");
    Serial.println("* is your wiring correct?");
    Serial.println("* did you change the chipSelect pin to match your shield or module?");
    while (1);
  } else {
    Serial.println("Wiring is correct and a card is present.");
  }

  // print the type of card
  Serial.println();
  Serial.print("Card type:         ");
  switch (card.type()) {
    case SD_CARD_TYPE_SD1:
      Serial.println("SD1");
      break;
    case SD_CARD_TYPE_SD2:
      Serial.println("SD2");
      break;
    case SD_CARD_TYPE_SDHC:
      Serial.println("SDHC");
      break;
    default:
      Serial.println("Unknown");
  }

  // Now we will try to open the 'volume'/'partition' - it should be FAT16 or FAT32
  if (!volume.init(card)) {
    Serial.println("Could not find FAT16/FAT32 partition.\nMake sure you've formatted the card");
    while (1);
  }

  Serial.print("Clusters:          ");
  Serial.println(volume.clusterCount());
  Serial.print("Blocks x Cluster:  ");
  Serial.println(volume.blocksPerCluster());

  Serial.print("Total Blocks:      ");
  Serial.println(volume.blocksPerCluster() * volume.clusterCount());
  Serial.println();

  // print the type and size of the first FAT-type volume
  uint32_t volumesize;
  Serial.print("Volume type is:    FAT");
  Serial.println(volume.fatType(), DEC);

  volumesize = volume.blocksPerCluster();    // clusters are collections of blocks
  volumesize *= volume.clusterCount();       // we'll have a lot of clusters
  volumesize /= 2;                           // SD card blocks are always 512 bytes (2 blocks are 1KB)
  Serial.print("Volume size (Kb):  ");
  Serial.println(volumesize);
  Serial.print("Volume size (Mb):  ");
  volumesize /= 1024;
  Serial.println(volumesize);
  Serial.print("Volume size (Gb):  ");
  Serial.println((float)volumesize / 1024.0);

  Serial.println("\nFiles found on the card (name, date and size in bytes): ");
  root.openRoot(volume);

  // list all files in the card with date and size
  root.ls(LS_R | LS_DATE | LS_SIZE);
}

void loop(void) {
}
#endif


#endif // CZ_LOG_SD_ENABLED
