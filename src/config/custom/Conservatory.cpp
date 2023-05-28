#ifdef AW_SETUP_CONSERVATORY

#include "../../SoilMoistureSensor.h"
#include "../../PumpMonitor.h"
#include "../../utility/PinTypes.h"
#include "EEPROM.h"

/**
 * What MCU pin we are using to communicate with the multiplexer.
 * Also known as the multiplexer Z pin
 * This needs to be an analog capable pin (to use with analogRead(...)), so we can do sensor readings
 */
#define MCU_TO_MUX_ZPIN cz::MCUPin(28)

/**
 * What MCU pin to use to set  the s0..s2 pins of the mux
 */
#define MCU_TO_MUX_S0 cz::MCUPin(18)
#define MCU_TO_MUX_S1 cz::MCUPin(20)
#define MCU_TO_MUX_S2 cz::MCUPin(21)

/**
 * Pins of the IO Expander used to turn on/off the motors
 */
#define MCU_MOTOR0 cz::MCUPin(15)
#define MCU_MOTOR1 cz::MCUPin(14)
#define MCU_MOTOR2 cz::MCUPin(13)
#define MCU_MOTOR3 cz::MCUPin(12)
#define MCU_MOTOR4 cz::MCUPin(9)
#define MCU_MOTOR5 cz::MCUPin(8)
#define MCU_MOTOR6 cz::MCUPin(7)
#define MCU_MOTOR7 cz::MCUPin(6)

/**
 * The MCU pin used to power the capacitive soil moisture sensors.
 * To simplify the board design, we turn on the sensors with one pin
 */
#define MCU_SENSORS_POWER cz::MCUPin(2)

/**
 * Multiplexer pins used to read the soil moisture sensors
 */
#define MUX_MOISTURE_SENSOR0 cz::MultiplexerPin(7) // Y0
#define MUX_MOISTURE_SENSOR1 cz::MultiplexerPin(5) // ...
#define MUX_MOISTURE_SENSOR2 cz::MultiplexerPin(3)
#define MUX_MOISTURE_SENSOR3 cz::MultiplexerPin(0)
#define MUX_MOISTURE_SENSOR4 cz::MultiplexerPin(1)
#define MUX_MOISTURE_SENSOR5 cz::MultiplexerPin(2)


namespace cz
{

namespace
{
	constexpr int sensorsPerBoard = 6;
	constexpr IOExpanderPin vinPins[sensorsPerBoard] = 
	{
		IO_EXPANDER_VPIN_SENSOR0,
		IO_EXPANDER_VPIN_SENSOR1,
		IO_EXPANDER_VPIN_SENSOR2,
		IO_EXPANDER_VPIN_SENSOR3,
		IO_EXPANDER_VPIN_SENSOR4,
		IO_EXPANDER_VPIN_SENSOR5
	};

	constexpr MultiplexerPin dataPins[sensorsPerBoard] =
	{
		MUX_MOISTURE_SENSOR0,
		MUX_MOISTURE_SENSOR1,
		MUX_MOISTURE_SENSOR2,
		MUX_MOISTURE_SENSOR3,
		MUX_MOISTURE_SENSOR4,
		MUX_MOISTURE_SENSOR5
	};

	constexpr IOExpanderPin motorPins[sensorsPerBoard] = 
	{
		IO_EXPANDER_MOTOR0,
		IO_EXPANDER_MOTOR1,
		IO_EXPANDER_MOTOR2,
		IO_EXPANDER_MOTOR3,
		IO_EXPANDER_MOTOR4,
		IO_EXPANDER_MOTOR5
	};

}

class GreenhouseSetup : public Setup
{
  public:

	virtual void begin() override
	{
		static_assert(IO_EXPANDER_ADDR>=0x0 && IO_EXPANDER_ADDR<=0x7, "Wrong macro value");

		// Initialize the i2c boards
		for(int i = 0; i< MAX_NUM_I2C_BOARDS; i++)
		{
			I2CBoard& board = m_i2cBoards[i];
			board.ioExpander.begin(IO_EXPANDER_ADDR + i);
			board.mux.begin(
				board.ioExpander,
				IO_EXPANDER_TO_MUX_S0,
				IO_EXPANDER_TO_MUX_S1,
				IO_EXPANDER_TO_MUX_S2,
				MCU_TO_MUX_ZPIN,
				IO_EXPANDER_TO_MUX_ENABLE);

		}
	}

	SoilMoistureSensor* createSoilMoistureSensor(int index)
	{
		MuxInterface& mux = m_i2cBoards[index < sensorsPerBoard ? 0 : 1].mux;
		DigitalOutputPin* vinPin = new MCP23xxxOutputPin(
			m_i2cBoards[index / sensorsPerBoard].ioExpander,
			vinPins[index % sensorsPerBoard].raw);
		MuxAnalogInputPin* dataPin = new MuxAnalogInputPin(
			m_i2cBoards[index / sensorsPerBoard].mux,
			dataPins[index % sensorsPerBoard].raw);
		return new SoilMoistureSensor(index, *vinPin, *dataPin);
	}
	
	PumpMonitor* createPumpMonitor(int index)
	{
		DigitalOutputPin* pin = new MCP23xxxOutputPin(
			m_i2cBoards[index / sensorsPerBoard].ioExpander,
			motorPins[index % sensorsPerBoard].raw);
		return new PumpMonitor(index, *pin);
	}

	struct I2CBoard
	{
	#if AW_MOCK_COMPONENTS
		MockMCP23017Wrapper ioExpander;
	#else
		MCP23017Wrapper ioExpander;
	#endif
		Mux8Channels mux;
	} m_i2cBoards[MAX_NUM_I2C_BOARDS];

};

Setup* createSetupObject_GreenHouse()
{
	static GreenhouseSetup setup;
	return &setup;
}

}

#endif
