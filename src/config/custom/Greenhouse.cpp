
#include "../../SoilMoistureSensor.h"
#include "../../PumpMonitor.h"
#include "../../utility/PinTypes.h"

/**
 * Note. Internally it adds 0x20, which is the base address
 */
#define IO_EXPANDER_ADDR 0x0

/**
 * What arduino pin we are using to communicate with the multiplexer.
 * Also known as the multiplexer Z pin
 * This needs to be an analog capable pin (to use with analogRead(...)), so we can do sensor readings
 */
#define MCU_TO_MUX_ZPIN cz::MCUPin(28)

/**
 * Pins of the IO expander to use to set the s0..s2 pins of the mux
 */
#define IO_EXPANDER_TO_MUX_S0 cz::IOExpanderPin(0+0)
#define IO_EXPANDER_TO_MUX_S1 cz::IOExpanderPin(0+1)
#define IO_EXPANDER_TO_MUX_S2 cz::IOExpanderPin(0+2)

/**
 * Pins of the IO Expander used to turn on/off the motors
 */
#define IO_EXPANDER_MOTOR0 cz::IOExpanderPin(0+3)
#define IO_EXPANDER_MOTOR1 cz::IOExpanderPin(0+4)
#define IO_EXPANDER_MOTOR2 cz::IOExpanderPin(0+5)
#define IO_EXPANDER_MOTOR3 cz::IOExpanderPin(0+6)
#define IO_EXPANDER_MOTOR4 cz::IOExpanderPin(0+7)
#define IO_EXPANDER_MOTOR5 cz::IOExpanderPin(8+6)

/**
 * Pins of the IO Expander used to power the capacitive soil moisture sensors
 */
#define IO_EXPANDER_VPIN_SENSOR0 cz::IOExpanderPin(8+5)
#define IO_EXPANDER_VPIN_SENSOR1 cz::IOExpanderPin(8+4)
#define IO_EXPANDER_VPIN_SENSOR2 cz::IOExpanderPin(8+3)
#define IO_EXPANDER_VPIN_SENSOR3 cz::IOExpanderPin(8+2)
#define IO_EXPANDER_VPIN_SENSOR4 cz::IOExpanderPin(8+1)
#define IO_EXPANDER_VPIN_SENSOR5 cz::IOExpanderPin(8+0)

/*
 * Pin from the IO Expander to the Mux's E pin.
 * This allows us to turn off the Mux, so multiple sensor boards can share a single sensor reading pin.
 * LOW - Mux enabled
 * HIGH - Mux disabled
 */
#define IO_EXPANDER_TO_MUX_ENABLE cz::IOExpanderPin(8+7)

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

	virtual const char* getName() const override
	{
		return "greenhouse";
	}

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