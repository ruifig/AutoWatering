#pragma once

#include <type_traits>
#include <assert.h>

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

private:
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
		assert(capacity > 1);
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
		assert(ret);
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
		assert(index < size());
		return m_data[(m_head + index) % m_capacity];
	}

	Type& getAtIndex(int index)
	{
		assert(index < size());
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


#if 0
namespace
{

template<class Q>
void printQueue(Q& q)
{
	printf("Elems=%d\n", q.size());
	for (int i = 0; i < q.size(); i++)
	{
		printf("%d, ", q.getAtIndex(i));
	}
	printf("\n");
}

template<class Q>
bool equals(Q& q, const std::vector<typename Q::Type>& v)
{
	if (q.size() != v.size())
		return false;

	for (int i = 0; i < q.size(); i++)
	{
		if (q.getAtIndex(i) != v[i])
			return false;
	}

	return true;
}

void runTests()
{
	TStaticFixedCapacityQueue<int, 5> q;

	printQueue(q);
	assert(q.push(0));
	assert(q.push(1));
	assert(q.push(2));
	assert(q.push(3));
	assert(q.push(4));

	assert(equals(q, { 0,1,2,3,4 }));
	printQueue(q);
	assert(q.push(5) == false);
	printQueue(q);


	assert(q.pop() == 0);
	assert(q.pop() == 1);
	printQueue(q);
	assert(equals(q, { 2,3,4 }));

	assert(q.push(5));
	printQueue(q);
	assert(equals(q, { 2,3,4,5 }));

	assert(q.push(2));
	assert(equals(q, { 2,3,4,5,2 }));
	assert(q.remove(2) == 2);
	assert(equals(q, { 3,4,5 }));

	assert(q.front() == 3);
	assert(q.back() == 5);

	q.clear();
	assert(q.size() == 0);


}

}

#endif

} // namespace cz

