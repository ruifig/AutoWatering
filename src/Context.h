#pragma once

#include <Arduino.h>
#include "utility/MCP23017Wrapper.h"
#include "utility/MuxNChannels.h"
#include <crazygaze/micromuc/Queue.h>
#include "AT24C.h"
#include "crazygaze/micromuc/MathUtils.h"

namespace cz
{
	struct SensorReading
	{
		enum Status : uint8_t
		{
			First,

			// Reading is to be considered valid
			Valid = First,

			// Samples were too random (aka: Standard Deviation too high), which means that
			// probably there no sensor attached and the data pin is floating
			NoSensor,

			// When a sensor is attached but for some reason is not getting power, it will consistently
			// return really low values. In those cases
			NoPower,

			Last = NoPower
		};


		SensorReading() = default;

		explicit SensorReading(unsigned int meanValue, float standardDeviation)
			: meanValue(meanValue)
			, standardDeviation(standardDeviation)
		{
			if (meanValue < AW_MOISTURESENSOR_ACCEPTABLE_MIN_VALUE)
			{
				status = Status::NoPower;
			}
			else if (standardDeviation > AW_MOISTURESENSOR_ACCEPTABLE_STANDARD_DEVIATION)
			{
				status = Status::NoSensor;
			}
			else
			{
				status = Status::Valid;
			}
		}

		bool isValid() const
		{
			return status==Status::Valid;
		}

		const char* getStatusText() const
		{
			static const char* strs[3] =
			{
				"Valid",
				"No Sensor Detected",
				"Sensor has no power"
			};

			return strs[status];
		}

		Status status = Status::Valid;
		unsigned int meanValue = 0;
		float standardDeviation = 0;
	};

	struct GraphPoint
	{
		// 0..100 moisture level
		unsigned int val : AW_GRAPH_POINT_NUM_BITS;
		// Tells if the motor was on at this point
		bool motorOn : 1;

		SensorReading::Status status : 2;
	} __attribute((packed));

	static_assert(sizeof(GraphPoint)==1, "GraphPoint size must be 1");

	// We set space for AW_GRAPH_NUMPOINT+1 to make it easier to deal with the display scrolling.
	// Considering there was just 1 update to the queue since the last draw, we can do the following:
	// The first point to draw is index 1, and to draw index 1, we erase the pixel in that pixel with the info from index 0
	using HistoryQueue = TStaticFixedCapacityQueue<GraphPoint, AW_GRAPH_NUMPOINTS + 1>;


	// Data that should be saved/loaded
	// NOTE: No members in this struct should raise any events, because this is also used in the UI to hold
	// temporary configs while in the settings menu.
	class GroupConfig
	{
	  private:

		//
		// Data that should be save/loaded
		struct SaveData
		{
			// Tells if this group is currently running
			bool running = false;
			// Sensor sampling interval in seconds
			unsigned int samplingInterval = AW_MOISTURESENSOR_DEFAULT_SAMPLINGINTERVAL;
			// Motor shot duration in seconds
			unsigned int shotDuration = AW_SHOT_DEFAULT_DURATION;

			// The sensor values decreases as moisture increases. (High Value = Dry, Low Value = Wet)
			// Air and water values are calculated automatically as sensor values are provided. This means the user wipe clean the sensor
			// to set the air value, and then submerse the sensor to set the water value.
			// Having air and water properly is not a requirement for things to work, since what matter is that the system know what 
			// sensor value is the threshold to turn on/off the motor
			#define START_AIR_VALUE 370
			#define START_WATER_VALUE (START_AIR_VALUE-5)
			unsigned int airValue = START_AIR_VALUE;
			unsigned int waterValue = START_WATER_VALUE;

			// Value above which irrigation should be turned on
			// NOTE: ABOVE because higher values means drier.
			// Using a big value as initial value, which means it will not turn on the motor until things are setup properly
			unsigned int thresholdValue = 65535;
		} m_data;

		// This needs to start as true, because:
		//	* Boot screen will save the config. If it was false, then "save" in the boot screen would skip stuff internally instead of actualy writting the data
		//		* When saving, this will be reset to false as part of "save()"
		//	* If the user chooses to let the boot menu load the current saved config, then this will be reset to false as part of "load()"
		mutable bool m_isDirty = true;

		// Current sensor value
		// This doesn't need to be saved or loaded
		unsigned int m_currentValue = START_AIR_VALUE - (START_AIR_VALUE-START_WATER_VALUE)/2;

		// Number of sensor readings done
		uint32_t m_numReadings = 0;

		struct
		{
			unsigned int minValue;
			unsigned int maxValue;
			bool enabled = false;
		} m_calibration;
		
	  public:

		int getSaveSize() const
		{
			return sizeof(m_data);
		}

	  	void log() const;
		void save(AT24C::Ptr& dst) const;
		void load(AT24C::Ptr& src);
		bool isDirty() const;
		bool isRunning() const;
		void setRunning(bool running);

		unsigned int getThresholdValue() const;
		unsigned int getThresholdValueAsPercentage() const;
		float getThresholdValueOnePercent() const;
		void setThresholdValue(unsigned int value);
		void setThresholdValueAsPercentage(unsigned int percentageValue);

		unsigned int getCurrentValue() const;
		unsigned int getCurrentValueAsPercentage() const;

		unsigned int getAirValue() const;
		unsigned int getWaterValue() const;

		/**
		 * Gets the sampling interval in seconds
		 */
		unsigned int getSamplingInterval() const;

		/**
		 * The UI needs the sampling interval in minutes
		 * When the sampling interval is < 1 min, it will return 0. THIS IS INTENTIONAL, so
		 * we let the UI specify minutes, but treat 0 as shortest possible (1 second)
		 */
		unsigned int getSamplingIntervalInMinutes() const;

		/**
		 * Sets the sampling interval (in seconds)
		 * Since the UI deals with this in minutes, we can allow the UI to also allow a value of 0, which
		 * internally gets clamped to 1 second. This is intentional
		 */
		void setSamplingInterval(unsigned int value_);

		/**
		 * Returns the water shot duration in seconds
		 */
		unsigned int getShotDuration() const;

		/**
		 * Set the water shot duration in seconds
		 */
		void setShotDuration(unsigned int value_);

		/**
		 * Changes the sensor value.
		 * \param currentValue The sensor value to save
		 * \param adjustRange If true, then air and water values are ajusted to accomodate for the "currentValue" parameter
		 */
		void setSensorValue(unsigned int currentValue_, bool adjustRange);

		/**
		 * Resets the air/water values to the default values, which is a very narrow range so it allows a new sensor range to be detected properly
		 */
		void startCalibration();
		void endCalibration();

	};

	class GroupData
	{
	  public:
		GroupData()
		{
		}
		
		void begin(uint8_t index);

		void logConfig() const
		{
			CZ_LOG(logDefault, Log, F("Group %u config:"), (unsigned int)m_index);
			m_cfg.log();
		}

		void setMoistureSensorValues(const SensorReading& sample);

		void setMotorState(bool state);
		bool isMotorOn() const;

		bool isRunning() const
		{
			return m_cfg.isRunning();
		}

		// Turns this group on or off
		// On : It will monitor and water
		// Off : Stops monitoring and watering
		void setRunning(bool state);

		unsigned int getThresholdValue() const
		{
			return m_cfg.getThresholdValue();
		}

		unsigned int getThresholdValueAsPercentage() const
		{
			return m_cfg.getThresholdValueAsPercentage();
		}

		//
		// Returns how much the threshold needs to change to show up as a 1% difference
		// This is used by the calibration menu, to allow for manually setting the threshold range
		float getThresholdValueOnePercent()
		{
			return m_cfg.getThresholdValueOnePercent();
		}

		void setThresholdValue(unsigned int value)
		{
			m_cfg.setThresholdValue(value);
		}

		void setThresholdValueAsPercentage(unsigned int percentageValue)
		{
			m_cfg.setThresholdValueAsPercentage(percentageValue);
		}

		unsigned int getCurrentValue() const
		{
			return m_cfg.getCurrentValue();
		}

		unsigned int getCurrentValueAsPercentage() const
		{
			return m_cfg.getCurrentValueAsPercentage();
		}

		unsigned int getAirValue() const
		{
			return m_cfg.getAirValue();
		}

		unsigned int getWaterValue() const
		{
			return m_cfg.getWaterValue();
		}

		unsigned int getSamplingInterval() const
		{
			return m_cfg.getSamplingInterval();
		}

		unsigned int getSamplingIntervalInMinutes() const
		{
			return m_cfg.getSamplingIntervalInMinutes();
		}

		void setSamplingInterval(unsigned int value)
		{
			m_cfg.setSamplingInterval(value);
		}

		unsigned int getShotDuration() const
		{
			return m_cfg.getShotDuration();
		}

		void setShotDuration(unsigned int value)
		{
			m_cfg.setShotDuration(value);
		}

		const HistoryQueue& getHistory()
		{
			return m_history;
		}

		uint8_t getIndex() const
		{
			return m_index;
		}

		void resetHistory();

		uint32_t getSensorErrorCount() const
		{
			return m_sensorErrors;
		}

		/**
		 * Returns a copy of the group config.
		 * This can be used by the UI for the settings menu, so it can change values without triggering events
		 */
		GroupConfig copyConfig() const
		{
			return m_cfg;
		}

		void setTo(GroupConfig cfg)
		{
			m_cfg = cfg;
		}

		// Sets this group as being configured or not at the moment.
		// When set to being configured, the following happens:
		//		* Sampling interval is temporarily set to AW_MOISTURESENSOR_CALIBRATION_SAMPLINGINTERVAL
		//		* Temporarily switches to raising SoilMoistureSensorCalibration_XXX events, instead of SoilMoistureSensor_XXX events
		void setInConfigMenu(bool inMenu)
		{
			m_inConfigMenu = inMenu;
		}

		bool isInConfigMenu() const
		{
			return m_inConfigMenu;
		}

	protected:
		friend class ProgramData;
		void save(AT24C::Ptr& dst, bool saveConfig, bool saveHistory) const;
		void load(AT24C::Ptr& src, bool loadConfig, bool loadHistory);
		int getConfigSaveSize() const
		{
			return m_cfg.getSaveSize();
		}

	  private:

		// How many seconds to wait between samplings
		uint8_t m_index;

		// Data that should be saved/loaded
		GroupConfig m_cfg;

		HistoryQueue m_history;
		
		uint32_t m_sensorErrors = 0;

		bool m_motorIsOn = false;
		// Used so we can detect when the motor was turned on and off before a sensor data point is inserted, so we can
		// add the motor flag to the next sensor data point when that happens.
		bool m_pendingMotorPoint = false;

		bool m_inConfigMenu = false;
	};

struct Context;

class ProgramData
{
public:
	ProgramData(Context& outer);

	GroupData& getGroupData(uint8_t index);

	// We only allow 1 sensor to be active at one give time, so we use this as a kind of mutex
	bool tryAcquireMuxMutex();
	void releaseMuxMutex();

	void save() const;
	
	// Saves just 1 single group's config (and not the history
	void saveGroupConfig(uint8_t index);
	void load();

	void begin();

	void logConfig() const;

	//
	// Returns true if a group is selected, false otherwise
	bool hasGroupSelected() const;

	//
	// Returns the selected group
	// nullptr if no group is selected
	GroupData* getSelectedGroup();

	//
	// Returns the selected group index or -1 if no group selected
	int getSelectedGroupIndex()
	{
		return m_selectedGroup;
	}

	// Tries to sets the selected group
	// Depending on the state of the program, it might fail, such as if we are in menus
	// -1 means no group selected
	bool trySetSelectedGroup(int8_t index);

	// Sets the temperarture reading in Celcius
	void setTemperatureReading(float temperatureC);
	// Set the humidity reading (0..100)
	void setHumidityReading(float humidity);

	float getTemperatureReading() const { return m_temperature; }
	float getHumidityReading() const { return m_humidity; }
	
  private:
	Context& m_outer;
	GroupData m_group[AW_MAX_NUM_PAIRS];
	bool m_muxMutex = false;
	int8_t m_selectedGroup = -1;

	// Temperature in Celcius
	float m_temperature = -100.0f;
	// Relative humiditity
	float m_humidity = -100.0f;
};

struct Context
{
	Context()
		: data(*this)
		, eeprom(0)
	{
	}

	void begin();

	ProgramData data;
	AT24C256 eeprom;
};

extern Context gCtx;

} // namespace cz
