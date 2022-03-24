#include "Config.h"
#include "MCP23017Wrapper.h"

namespace cz
{

namespace detail
{
	extern const uint8_t muxChannel[16][4];
	// Base class for 74HC4051 8-Channel-Mux  or 74HC4067 16-Channel-Mux 
	template<int NUM_SPINS>
	class BaseMux
	{
	public:

		// Constructor for an 8 channel mux
		BaseMux(MCP23017WrapperInterface& ioExpander, IOExpanderPin s0, IOExpanderPin s1, IOExpanderPin s2, MCUPin z)
			: m_ioExpander(ioExpander)
			, m_sPins{s0, s1, s2}
			, m_zPin(z)
		{
		}

		// Constructor for an 16 channel mux
		BaseMux(MCP23017WrapperInterface& ioExpander, IOExpanderPin s0, IOExpanderPin s1, IOExpanderPin s2, IOExpanderPin s3, MCUPin z)
			: m_ioExpander(ioExpander)
			, m_sPins{s0, s1, s2, s3}
			, m_zPin(z)
		{
		}

		MCUPin getMCUZPin() const
		{
			return m_zPin;
		}

		void begin()
		{
			pinMode(m_zPin.raw, m_zPinMode);
			for (auto&& pin : m_sPins)
			{
				m_ioExpander.pinMode(pin, OUTPUT);
			}
		}

		/**
		 * Sets the channel to use, and sets the MCU's pinMode to unknown,
		 * so external code can make use of it directly.
		 */
		void setChannel(MultiplexerPin channel)
		{
			setChannelImpl(channel);
			m_zPinModeFlag = 255;
		}

		/**
		 * Sets the mux channel
		 */
		int analogRead(MultiplexerPin channel, PinMode zPinMode = INPUT)
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

	protected:
		void setChannelImpl(MultiplexerPin channel)
		{
			// Set s0-sN
			for (uint8_t i = 0; i < NUM_SPINS ; i++)
			{
				m_ioExpander.digitalWrite(m_sPins[i], muxChannel[channel.raw][i]);
			}

			// Seems like I need this delay after setting the channel ?
			// In some occasions if the pinMode changed externally and is set above, the first call to this function
			// after the external use of the MCU's pin wouldn't give the right result.
			//delayMicroseconds(70);
			delayMicroseconds(1);
		}

		MCP23017WrapperInterface& m_ioExpander;
		IOExpanderPin m_sPins[NUM_SPINS];
		MCUPin m_zPin;

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

	Mux8Channels(MCP23017WrapperInterface& ioExpander, IOExpanderPin s0, IOExpanderPin s1, IOExpanderPin s2, MCUPin z) : BaseMux(ioExpander, s0, s1, s2, z) {}
};

class Mux16Channels : public detail::BaseMux<4>
{
  public:
	Mux16Channels(MCP23017WrapperInterface& ioExpander, IOExpanderPin s0, IOExpanderPin s1, IOExpanderPin s2, IOExpanderPin s3, MCUPin z) : BaseMux(ioExpander, s0, s1, s2, s3, z) {}
};

} // namespace cz
