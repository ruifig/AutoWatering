#pragma once

#include "crazygaze/micromuc/Logging.h"

namespace cz
{

/**
 * Reads a string from Serial, without using the String or memory allocation
 * The sender needs to write \r\n at the end of each string.
 */
template<int MaxSize=64>
class SerialStringReader
{
public:
	/**
	 * Tries to read a string without blocking
	 * A string is considered full when a \r\n is detected
	 * @return Pointer to the string, or nullptr if no string detected
	 */
	bool tryRead()
	{
		int ch = Serial.read();
		while(ch != -1)
		{
			if (ch == '\n')
			{
				m_buf[m_index] = 0;
				return true;
			}
			else if (ch == '\r')
			{
				// drop this one
			}
			else
			{
				m_buf[m_index++] = ch;
				if (m_index == MaxSize-1)
				{
					m_buf[m_index] = 0;
					return true;
				}
			}

			ch = Serial.read();
		}

		return false;
	}

	const char* retrieve()
	{
		m_index = 0;
		return m_buf;
	}

private:
	char m_buf[MaxSize];
	int m_index = 0;
};


} // namespace cz

