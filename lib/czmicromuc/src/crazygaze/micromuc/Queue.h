#pragma once

#include "crazygaze/micromuc/czmicromuc.h"
#include <type_traits>

namespace cz
{

/**
 * Fixed capacity queue.
 * It expects the user to specify a buffer to use.
 * Also, it wastes 1 slot so it can detect if the queue is empty or full
 */
template<typename T>
class TFixedCapacityQueue
{
public:
	using Type = T;
	static_assert(std::is_pod<Type>::value, "Type must be a POD");

protected:
	T* m_data;
	int m_capacity;
	int m_tail; // write position
	int m_head; // read position

public:

	/**
	 * @param buffer Buffer to use to implement the queue
	 * @param capacity How many elements fit in the buffer. Please note that queue wastes 1 slot, so the real queue capacity
	 * will capacity - 1
	 */
	TFixedCapacityQueue(Type* buffer, int capacity)
	{
		CZ_ASSERT(capacity > 1);
		m_data = buffer;
		m_capacity = capacity;
		m_tail = 0;
		m_head = 0;
	}

	bool isEmpty() const
	{
		return m_tail == m_head;
	}

	bool isFull() const
	{
		return size() == m_capacity - 1;
	}

	int size() const
	{
		return (m_tail + m_capacity - m_head) % m_capacity;
	}

	int capacity() const
	{
		return m_capacity;
	}

	bool push(const Type& val)
	{
		if (isFull())
		{
			return false;
		}

		m_data[m_tail] = val;
		m_tail = (m_tail + 1) % m_capacity;
		return true;
	}

	bool pop(Type& outVal)
	{
		if (isEmpty())
		{
			return false;
		}

		outVal = m_data[m_head];
		m_head = (m_head + 1) % m_capacity;
		return true;
	}

	Type pop()
	{
		Type val;
		bool ret = pop(val);
		CZ_ASSERT(ret);
		return val;
	}

	bool peek(Type& outVal) const
	{
		if (isEmpty())
		{
			return false;
		}

		outVal = m_data[m_head];
		return true;
	}

	void clear()
	{
		m_head = m_tail = 0;
	}

	/**
	 * Removes from the queue all items matching the specified value
	 * @return number of items removed
	 */
	int remove(const Type& val)
	{
		int count = 0;
		int todo = size();
		int dstIndex = m_head;
		int srcIndex = m_head;

		while (todo--)
		{
			if (m_data[srcIndex] == val) {
				count++;
			}
			else {
				m_data[dstIndex] = m_data[srcIndex];
				dstIndex = (dstIndex + 1) % m_capacity;
			}
			srcIndex = (srcIndex + 1) % m_capacity;
		}

		m_tail = dstIndex;
		return count;
	}

	const Type& getAtIndex(int index) const
	{
		CZ_ASSERT(index < size());
		return m_data[(m_head + index) % m_capacity];
	}

	Type& getAtIndex(int index)
	{
		CZ_ASSERT(index < size());
		return m_data[(m_head + index) % m_capacity];
	}

	const Type& front() const
	{
		return getAtIndex(0);
	}

	Type& front()
	{
		return getAtIndex(0);
	}

	const Type& back() const
	{
		return getAtIndex(size()-1);
	}

	Type& back()
	{
		return getAtIndex(size()-1);
	}

	/**
	 * Checks if the given value is in the queue
	 */
	bool find(const Type& val) const
	{
		for(int i=0; i<size(); i++)
		{
			if (getAtIndex(i) == val)
			{
				return true;
			}
		}
		return false;
	}

};

template<typename T, int SIZE>
class TStaticFixedCapacityQueue : public TFixedCapacityQueue<T>
{
public:
	using Type = T;
	Type m_buffer[SIZE + 1]; // Using +1 because TFixedCapacityQueue wastes 1 slot

	TStaticFixedCapacityQueue() : TFixedCapacityQueue<T>(m_buffer, SIZE + 1)
	{
	}
};

void runQueueTests();


} // namespace cz

