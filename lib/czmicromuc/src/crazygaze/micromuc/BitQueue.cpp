#include "BitQueue.h"
#include "Logging.h"
#include <algorithm>
#include <assert.h>

static inline size_t stack_size()
{
    return RAMEND - SP;
}

#define CZ_TEST(expression) if (!(expression)) { ::cz::_doAssert(__FILENAME__, __LINE__, #expression); }

#define BIT_MASK(H,L) (((unsigned) -1 >> (31 - (H))) & ~((1U << (L)) - 1))

/*
	Gets the bits between H-L (inclusive)
	For example, to get the top 4 bits of some byte variable:
		GET_BITS(somevar, 7, 4)
*/
#define GET_BITS(val, H, L) \
	((val & (unsigned int)((((unsigned int)(1) << ((H)-(L)+1)) -1) << (L))) >> (L))

#define IS_BIT_SET(val, B) \
	((val) & ((unsigned int)1 << (unsigned int)(B)))

#define SET_BIT(var, pos, value) \
	((var & ~((uint32_t)(1) << pos)) | ((uint32_t)(value)<<pos))

// Makes a bit mask for a range of bits H-L (inclusive)
#define MAKE_MASK(H,L) \
	((((unsigned int)(1) << ((H)-(L)+1)) -1) << (L))

// Sets a range of bits to zero. H-L (inclusive)
#define ZERO_BITS(var, H, L) \
	((var) & (~MAKE_MASK((H),(L))))
	
// Sets a range of bits (H-L, inclusive) to the specified value "val"
#define SET_BITS(var, H, L, val) \
	( ZERO_BITS((var),(H),(L)) | ((val) << (L)) )

namespace cz
{

namespace
{

	/**
	 * Helper class to make it easier to write bits to a buffer
	 */
	struct WriteHelper
	{
		uint8_t* ptr;
		unsigned int pos;
		void write(uint8_t val, unsigned int numBits)
		{
			while (numBits)
			{
				unsigned int idx = pos / 8;
				unsigned int L = pos % 8;
				unsigned int todo = std::min(numBits, 8 - L);
				unsigned int H = L + todo - 1;
				ptr[idx] = SET_BITS(ptr[idx], H, L, val & BIT_MASK(todo -1, 0));
				val >>= todo;
				numBits -= todo;
				pos += todo;
			}
		}
	};

}

FixedCapacityBitQueue::FixedCapacityBitQueue(uint8_t* buffer, int capacityBits)
{
	assert(capacityBits >= 2);
	m_data = buffer;
	m_capacity = capacityBits;
	m_tail = 0;
	m_head = 0;
}

bool FixedCapacityBitQueue::isEmpty() const
{
	return m_tail == m_head;
}

bool FixedCapacityBitQueue::isFull() const
{
	return size() == m_capacity - 1;
}

unsigned int FixedCapacityBitQueue::capacity() const
{
	return m_capacity - 1;
}

unsigned int FixedCapacityBitQueue::size() const
{
	return (m_tail + m_capacity - m_head) % m_capacity;
}

unsigned int FixedCapacityBitQueue::availableCapacity() const
{
	return (m_capacity - 1) - size();
}

bool FixedCapacityBitQueue::pushBits(uint8_t bits, unsigned int numBits)
{
	if (availableCapacity() < numBits)
		return false;

	pushImpl(bits, numBits);
	return true;
}

bool FixedCapacityBitQueue::pushBits(const uint8_t* src, unsigned int numBits)
{
	if (availableCapacity() < numBits)
		return false;

	while (numBits)
	{
		unsigned int todo = std::min(numBits, 8U);
		pushImpl(*src, todo);
		src++;
		numBits -= todo;
	}
	return true;
}

void FixedCapacityBitQueue::forcePushBits(uint8_t bits, unsigned int numBits)
{
	unsigned int available = availableCapacity();
	if (available < numBits)
	{
		dropBits(numBits - available);
	}
	pushImpl(bits, numBits);
}

void FixedCapacityBitQueue::forcePushBits(const uint8_t* src, unsigned int numBits)
{
	unsigned int available = availableCapacity();
	if (available < numBits)
	{
		dropBits(numBits - available);
	}

	while (numBits)
	{
		unsigned int todo = std::min(numBits, 8U);
		pushImpl(*src, todo);
		src++;
		numBits -= todo;
	}
}

void FixedCapacityBitQueue::dropBits(unsigned int numBits)
{
	{
		unsigned int s = size();
		if (s < numBits)
			numBits = s;
	}

	while (numBits)
	{
		unsigned int idx = m_head / 8;
		unsigned int L = m_head % 8;
		unsigned int todo = std::min(numBits, std::min(8 - L, m_capacity - m_head));
#if BITQUEUE_ZERO_ONPOP
		unsigned int H = L + todo - 1;
		m_data[idx] = ZERO_BITS(m_data[idx], H, L);
#endif
		numBits -= todo;
		m_head = (m_head + todo) % m_capacity;
	};
}

unsigned int FixedCapacityBitQueue::pop(uint8_t* dst, unsigned int numBits)
{
	unsigned int ret;
	{
		unsigned int s = size();
		if (s < numBits)
			numBits = s;
	}
	ret = numBits;

	WriteHelper writer{ dst, 0 };

	while (numBits)
	{
		unsigned int idx = m_head / 8;
		unsigned int L = m_head % 8;
		unsigned int todo = std::min(numBits, std::min(8 - L, m_capacity - m_head));
		unsigned int H = L + todo - 1;
		uint8_t val = GET_BITS(m_data[idx], H, L);
#if BITQUEUE_ZERO_ONPOP
		m_data[idx] = ZERO_BITS(m_data[idx], H, L);
#endif
		writer.write(val, todo);
		numBits -= todo;
		m_head = (m_head + todo) % m_capacity;
	};

	return ret;
}

void FixedCapacityBitQueue::clear()
{
	m_head = m_tail = 0;
#if BITQUEUE_ZERO_ONPOP
	auto numBytes = (m_capacity / 8) + ((m_capacity % 8) ? 1 : 0);
	memset(m_data, 0, numBytes);
#endif
}

bool FixedCapacityBitQueue::getAtIndex(unsigned int index, uint8_t* dst, unsigned int numBits) const
{
	if (numBits > size())
	{
		return false;
	}

	WriteHelper writer{ dst, 0 };

	unsigned int head = (m_head + index) % m_capacity;

	while (numBits)
	{
		unsigned int idx = head / 8;
		unsigned int L = head % 8;
		unsigned int todo = std::min(numBits, std::min(8 - L, m_capacity - head));
		unsigned int H = L + todo - 1;
		uint8_t val = GET_BITS(m_data[idx], H, L);
		writer.write(val, todo);
		numBits -= todo;
		head = (head + todo) % m_capacity;
	};

	return true;
}

void FixedCapacityBitQueue::pushImpl(uint8_t val, unsigned int numBits)
{
	while (numBits)
	{
		unsigned int idx = m_tail / 8;
		unsigned int L = m_tail % 8;
		unsigned int todo = std::min(numBits, std::min(8 - L, m_capacity - m_tail));
		unsigned int H = L + todo - 1;

#if BITQUEUE_ZERO_ONPOP
		// Unused bits should have been cleared when popping
		assert(GET_BITS(m_data[idx], H, L) == 0);
#endif
		m_data[idx] = SET_BITS(m_data[idx], H, L, val & MAKE_MASK(todo - 1, 0));
		m_tail = (m_tail + todo) % m_capacity;
		numBits -= todo;
		val >>= todo;
	}
}

} // namespace cz


//////////////////////////////////////////////////////////////////////////
// Tests
//////////////////////////////////////////////////////////////////////////
namespace cz
{
namespace
{

	struct BitArray
	{
		uint8_t* data;
		unsigned int bitPos;


		static unsigned int getBit(const uint8_t* data, unsigned int pos)
		{
			unsigned int idx = pos / 8;
			unsigned int bit = pos % 8;
			return IS_BIT_SET(data[idx], bit) ? 1 : 0;
		}

		static void setBit(uint8_t* data, unsigned int pos, unsigned int val)
		{
			unsigned int idx = pos / 8;
			unsigned int bit = pos % 8;
			data[idx] = SET_BIT(data[idx], bit, val);
		}

		void getBits(uint8_t* dst, unsigned int numBits)
		{
			unsigned int dstPos = 0;
			while (numBits--)
			{
				setBit(dst, dstPos, getBit(data, bitPos));
				dstPos++;
				bitPos++;
			}
		}

		void setBits(const uint8_t* src, unsigned int numBits)
		{
			unsigned int srcPos = 0;
			while (numBits--)
			{
				setBit(data, bitPos, getBit(src, srcPos));
				srcPos++;
				bitPos++;
			}
		}

	};

	template<unsigned QSIZE_BYTES>
	void runTests1()
	{
		Serial.print("1. Stack size = "); Serial.println(stack_size());
		// Fill the test data with: 1,2,3,...,255
		uint8_t testData[QSIZE_BYTES];
		for (int idx = 0; idx < QSIZE_BYTES; idx++)
		{
			testData[idx] = idx;
		}

		auto test1 = [&testData](unsigned int pushSize, unsigned int popSize, unsigned int offset, bool forcePush)
		{
			Serial.print("2. Stack size = "); Serial.println(stack_size());
			cz::TStaticFixedBitCapacityQueue<QSIZE_BYTES * 8> q;
			BitArray testArray{ testData, 0 };

			Serial.print("3. Stack size = "); Serial.println(stack_size());

			CZ_TEST(q.capacity() == QSIZE_BYTES * 8);
			CZ_TEST(q.availableCapacity() == QSIZE_BYTES * 8);
			unsigned int todo = offset;
			while (todo--)
			{
				q.pushBits(1, 1);
			}
			CZ_TEST(q.size() == offset);
			CZ_TEST(q.availableCapacity() == ((QSIZE_BYTES * 8) - offset));

			if (!forcePush)
			{
				q.dropBits(offset);
				CZ_TEST(q.size() == 0);
			}

			Serial.print("4. Stack size = "); Serial.println(stack_size());
			todo = QSIZE_BYTES * 8;
			while(todo)
			{
				Serial.print("5. Stack size = "); Serial.println(stack_size());
				Serial.flush();
				Serial.println(todo);
				Serial.flush();

				unsigned int size = std::min(todo, pushSize);
				Serial.println("  a");
				Serial.flush();

				uint8_t val;
				testArray.getBits(&val, size);
				Serial.println("  b");
				Serial.flush();

				if (forcePush)
				{
					q.forcePushBits(val, size);
					Serial.println("  c");
					Serial.flush();
				}
				else
				{
					bool ret = q.pushBits(val, size);
					Serial.println("  c");
					Serial.flush();
					CZ_TEST(ret);
					Serial.println("  cc");
					Serial.flush();
				}

				todo -= size;
			}
			CZ_TEST(q.size() == QSIZE_BYTES*8);
			CZ_TEST(q.isFull());
			CZ_TEST(q.pushBits(uint8_t(0), 1) == false);

			//
			// Try getAtIndex for all items
			//
			static_assert((QSIZE_BYTES % 2) == 0, "Must be even");
			for (unsigned int n = 0; n < QSIZE_BYTES; n+=2)
			{
				uint8_t ch[2];
				q.getAtIndex(n * 8, ch, 16);
				CZ_TEST(ch[0] == (uint8_t)n);
				CZ_TEST(ch[1] == (uint8_t)(n+1));
			}

			uint8_t retrievedData[QSIZE_BYTES];
			BitArray retrievedArray{ retrievedData, 0 };
			todo = QSIZE_BYTES * 8;
			while(todo)
			{
				unsigned size = std::min(todo, popSize);
				uint8_t val;
				q.pop(&val, size);
				retrievedArray.setBits(&val, size);
				todo -= size;
			}
			CZ_TEST(q.size() == 0);
			CZ_TEST(q.isEmpty());

			CZ_TEST(memcmp(testArray.data, retrievedArray.data, QSIZE_BYTES) == 0);
		};

		for (int a = 1; a < 8; a++)
		{
			for (int b = 1; b < 8; b++)
			{
				for (int c = 0; c < 16; c++)
				{
					test1(a, b, c, false);
					test1(a, b, c, true);
				}
			}
		}

	}

	template<unsigned NUM_ELEMS>
	void runTests2()
	{
		#define ELE_BITS 6

		//
		// Do some tests as if we are using a bitfield struct of 6 bits
		//
		struct Foo
		{
			unsigned int v : ELE_BITS;
		};

		auto test = [](unsigned int offset, bool forcePush)
		{
			cz::TStaticFixedBitCapacityQueue<NUM_ELEMS * ELE_BITS> q;

			unsigned int todo = offset;
			while (todo--)
			{
				q.pushBits(1, 1);
			}
			CZ_TEST(q.size() == offset);

			if (!forcePush)
			{
				q.dropBits(offset);
				CZ_TEST(q.size() == 0);
			}

			unsigned int count = 0;
			todo = NUM_ELEMS;
			while(todo--)
			{
				Foo v{ count };
				if (forcePush)
				{
					q.forcePushBits((uint8_t*)&v, ELE_BITS);
				}
				else
				{
					bool ret = q.pushBits((uint8_t*)&v, ELE_BITS);
					CZ_TEST(ret);
				}
				count++;
			}

			CZ_TEST((q.size() / ELE_BITS) == count);

			//
			// Try getAtIndex for all items
			//
			todo = NUM_ELEMS;
			count = 0;
			while (todo--)
			{
				Foo v{ 0x255 };
				q.getAtIndex(count * ELE_BITS, (uint8_t*)&v, ELE_BITS);
				CZ_TEST(v.v == (count & MAKE_MASK(ELE_BITS - 1, 0)));
				count++;
			}
		};

		test(1, false);
		test(1, true);


		for (int c = 0; c < 16; c++)
		{
			test(c, false);
			test(c, true);
		}
	}

} // anonymous namespace

void runBitQueueTests()
{
	Serial.print("Stack size = "); Serial.println(stack_size());
	CZ_LOG(logDefault, Log, F("Running BitQueueTests..."));
	Serial.print("Stack size = "); Serial.println(stack_size());
	runTests1<4>();
	Serial.print("Stack size = "); Serial.println(stack_size());
	runTests1<256>();
	runTests1<400>();
	runTests2<4>();
	runTests2<128>();
	CZ_LOG(logDefault, Log, F("Finished BitQueueTests"));

}

} // namespace cz
