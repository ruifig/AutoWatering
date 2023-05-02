/*
This file should only be included from the config/custom/<CONFIG.H> file being used.
*/
#pragma once

#if defined(DEBUG) || defined(_DEBUG)
	#define AW_DEBUG 1
#else
	#define AW_DEBUG 0
#endif

/*
To make things easier during development, setting this to 1 will cause some settings
to be tweaked to allow faster iterations.
For example, sensor intervals can be shortened.
*/
#ifndef AW_FASTER_ITERATION
	#if AW_DEBUG
		#define AW_FASTER_ITERATION 1
	#else
		#define AW_FASTER_ITERATION 0
	#endif
#endif

/*
To make things during development, setting this to 1 will use mock components for some things
This hasn't been used for a while, so not sure if working
*/
#ifndef AW_MOCK_COMPONENTS
	#define AW_MOCK_COMPONENTS 0
#endif

/**
 * How many bits to set the ADC readings to.
 * E.g: The RP2040 has a 12-bit ADC
 */
#define AW_ADC_NUM_BITS 12

#ifdef __cplusplus
	namespace cz
	{
		class SoilMoistureSensor;
		class PumpMonitor;

		/*
		A custom setup must implement this interface
		*/
		class Setup
		{
		  public:
			// Called for each sensor+motor pair
			virtual void begin() = 0;
			virtual SoilMoistureSensor* createSoilMoistureSensor(int index) = 0;
			virtual PumpMonitor* createPumpMonitor(int index) = 0;
		};

	} // namespace cz

#endif