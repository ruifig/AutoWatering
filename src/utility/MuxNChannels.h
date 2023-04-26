#include "PinTypes.h"
#include "MCP23017Wrapper.h"

namespace cz
{

class MuxInterface
{
  public:
	virtual MCUPin getMCUZPin() const = 0;
	virtual void setChannel(MultiplexerPin channel) = 0;
	virtual int analogRead(MultiplexerPin channel, PinMode zPinMode = INPUT) = 0;
	virtual void setEnabled(bool enabled) = 0;
};

namespace detail
{
	extern const uint8_t muxChannel[16][4];
	// Base class for a 74HC4051 8-Channel-Mux  or 74HC4067 16-Channel-Mux that is connected to an MCP23017
	template<int NUM_SPINS>
	class BaseMux : public MuxInterface
	{
	public:
		BaseMux()
			: m_sPins{ IOExpanderPin(0), IOExpanderPin(0), IOExpanderPin(0) }
			, m_zPin(MCUPin(0))
			, m_enablePin{IOExpanderPin(0)}
		{
		}

		// Begin for 8 channel mux
		void begin(MCP23017WrapperInterface& ioExpander, IOExpanderPin s0, IOExpanderPin s1, IOExpanderPin s2, MCUPin z, IOExpanderPin enable)
		{
			m_ioExpander = &ioExpander;
			m_sPins[0] = s0;
			m_sPins[1] = s1;
			m_sPins[2] = s2;
			m_zPin = z;
			m_enablePin = enable;
			doBegin();
		}

		// Begin for 8 channel mux
		void begin(MCP23017WrapperInterface& ioExpander, IOExpanderPin s0, IOExpanderPin s1, IOExpanderPin s2, IOExpanderPin s3, MCUPin z, IOExpanderPin enable)
		{
			m_ioExpander = &ioExpander;
			m_sPins[0] = s0;
			m_sPins[1] = s1;
			m_sPins[2] = s2;
			m_sPins[3] = s3;
			m_zPin = z;
			m_enablePin = enable;
			doBegin();
		}

		virtual MCUPin getMCUZPin() const override
		{
			return m_zPin;
		}

		/**
		 * Sets the channel to use, and sets the MCU's pinMode to unknown,
		 * so external code can make use of it directly.
		 */
		virtual void setChannel(MultiplexerPin channel) override
		{
			setChannelImpl(channel);
			m_zPinModeFlag = 255;
		}

		/**
		 * Sets the mux channel
		 */
		virtual int analogRead(MultiplexerPin channel, PinMode zPinMode = INPUT) override
		{
			if (m_zPinMode != zPinMode )
			{
				m_zPinMode = zPinMode;
				pinMode(m_zPin.raw, m_zPinMode);
				delayMicroseconds(70);
			}

			setChannelImpl(channel);

		#if MOCK_COMPONENTS
			return 0;
		#else
			return ::analogRead(m_zPin.raw);
		#endif
		}

		virtual void setEnabled(bool enabled) override
		{
			// LOW - enabled
			// HIGH - disabled
			m_ioExpander->digitalWrite(m_enablePin, enabled ? LOW : HIGH);
		}

	protected:
		void doBegin()
		{
			pinMode(m_zPin.raw, m_zPinMode);

			m_ioExpander->pinMode(m_enablePin, OUTPUT);
			for (auto&& pin : m_sPins)
			{
				m_ioExpander->pinMode(pin, OUTPUT);
			}

			setEnabled(false);
		}

		void setChannelImpl(MultiplexerPin channel)
		{
			if (m_currChannel == channel.raw)
			{
				return;
			}

			m_currChannel = channel.raw;

			// Set s0-sN
			for (uint8_t i = 0; i < NUM_SPINS ; i++)
			{
				m_ioExpander->digitalWrite(m_sPins[i], muxChannel[channel.raw][i]);
			}

			// Seems like I need this delay after setting the channel ?
			// In some occasions if the pinMode changed externally and is set above, the first call to this function
			// after the external use of the MCU's pin wouldn't give the right result.
			// One particular problem I noticed was when having 2 groups ON (A and B), where A actually has a sensor connector, and B doesn't,
			// B would read garbage as expected, but when reading A the first reading would be wrong to the point it will mostly significantly affect 
			// the calculated standard deviation
			delayMicroseconds(100);
		}

		MCP23017WrapperInterface* m_ioExpander = nullptr;
		IOExpanderPin m_sPins[NUM_SPINS];
		MCUPin m_zPin;
		IOExpanderPin m_enablePin;
		int m_currChannel = 255;

		union
		{
			PinMode m_zPinMode = INPUT;
			// If this is 255, then current pinMode is unknown
			uint8_t m_zPinModeFlag;
		};
	};

} // namespace detail

class Mux8Channels : public detail::BaseMux<3>
{
  public:
};

class Mux16Channels : public detail::BaseMux<4>
{
  public:
};

class MuxAnalogInputPin : public AnalogInputPin
{
  public:
	MuxAnalogInputPin(const MuxAnalogInputPin&) = delete;
	MuxAnalogInputPin& operator=(const MuxAnalogInputPin&) = delete;
	MuxAnalogInputPin(MuxInterface& outer, uint8_t pin)
		: m_outer(outer)
		, m_pin(pin)
	{
	}

	//
	// AnalogInputPin
	//
	virtual void enable() override
	{
		m_outer.setEnabled(true);
	}

	virtual int read() override
	{
		return m_outer.analogRead(MultiplexerPin(m_pin), PinMode::INPUT);
	}

	virtual void disable() override
	{
		m_outer.setEnabled(false);
	}

  private:
	MuxInterface& m_outer;
	uint8_t m_pin;
};

} // namespace cz
