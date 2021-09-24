#pragma once

#include "crazygaze/micromuc/czmicromuc.h"

#ifndef CZ_BITQUEUE_ZERO_ONPOP
	#define CZ_BITQUEUE_ZERO_ONPOP 0
#endif

namespace cz
{

/**
 * Fixed capacity bit queue.
 * It expects the user to specify a buffer to use.
 */
class FixedCapacityBitQueue
{

private:
	uint8_t* m_data;
	unsigned int m_capacity; // capacity in bits
	unsigned int m_tail; // write position in bits
	unsigned int m_head; // read position in bits

public:

	/**
	 * @param buffer Buffer to use to implement the queue
	 * @param capacity How many bits fit in the buffer. Please note that queue wastes 1 bit, so the real queue capacity
	 * will capacity - 1
	 */
	FixedCapacityBitQueue(uint8_t* buffer, int capacityBits);

	/**
	 * Tells if the queue is empty
	 */
	bool isEmpty() const;

	/**
	 * Tells if the queue is full
	 */
	bool isFull() const;

	/**
	 * Returns capacity in bits
	 */
	unsigned int capacity() const;

	/**
	 * Returns the queue size in bits
	 */
	unsigned int size() const;

	/** 
	 * Returns how many bits still available in the queue
	 */
	unsigned int availableCapacity() const;

	/**
	 * Pushes bits (maximum of 8) into the queue
	 * @return true on success, false if not enough free space in the queue to satisfy request
	 */
	bool pushBits(uint8_t bits, unsigned int numBits);

	/**
	 * Pushes bits (can be more than 8 bits) into the queue
	 * @return true on success, false if not enough free space in the queue to satisfy request
	 */
	bool pushBits(const uint8_t* src, unsigned int numBits);

	/**
	 * Forcibly pushes bits (maximum of 8) into the queue.
	 * What this means is that if there is not enough free space in the queue, it will drop bits automatically before 
	 * pushing.
	 */
	void forcePushBits(uint8_t bits, unsigned int numBits);

	/**
	 * Forcibly pushes bits (can be more than 8) into the queue.
	 * What this means is that if there is not enough free space in the queue, it will drop bits automatically before 
	 * pushing.
	 */
	void forcePushBits(const uint8_t* src, unsigned int numBits);

	/**
	 * Drops the specified number of bits.
	 * 
	 */
	void dropBits(unsigned int numBits);

	/**
	 * Pops an arbitrary number of bits
	 * @return number of bits popped
	 */
	unsigned int pop(uint8_t* dst, unsigned int numBits);

	/**
	 * Clears the queue
	 */
	void clear();

	/**
	 * Gets from the specified index into the buffer.
	 * Note that this is useful for peeking at the queue. It doesn't remove any bits from the queue.
	 * @return true if success, false if not enough bits in the queue to satisfy request
	 */
	bool getAtIndex(unsigned int index, uint8_t* dst, unsigned int numBits) const;

private:

	void pushImpl(uint8_t val, unsigned int numBits);

};

template<unsigned int SIZE_BITS>
class TStaticFixedBitCapacityQueue : public FixedCapacityBitQueue
{
public:
	/**
	 * Reserve space for requested bits + 1, because FixedCapacityBitQueue wastes 1 bit
	 */
	uint8_t m_buffer[((SIZE_BITS+1) / 8) + (((SIZE_BITS+1) % 8) ? 1 : 0)];
	TStaticFixedBitCapacityQueue() : FixedCapacityBitQueue(m_buffer, SIZE_BITS+1)
	{
	}

	void fixupDataAfterLoad()
	{
		m_data = m_buffer;
	}
};

void runBitQueueTests();

} // namespace cz

