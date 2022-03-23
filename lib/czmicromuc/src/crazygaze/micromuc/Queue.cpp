#include "Queue.h"

#include "Logging.h"
#include "StringUtils.h"
#include <algorithm>
#include <assert.h>
#include <initializer_list>

#define CZ_TEST(expression) if (!(expression)) { ::cz::_doAssert(__FILENAME__, __LINE__, F(#expression)); }

//////////////////////////////////////////////////////////////////////////
// Tests
//////////////////////////////////////////////////////////////////////////
namespace cz
{
namespace
{

	template<class Q>
	void printQueue(Q& q)
	{
		Serial.print("Elems="); Serial.println(q.size()); Serial.print("    ");
		for (int i = 0; i < q.size(); i++)
		{
			Serial.print(q.getAtIndex(i)); Serial.print(", ");
		}
		Serial.println();
	}

	template<class Q>
	bool __attribute__((noinline)) equals(Q& q, std::initializer_list<typename Q::Type> list)
	{
		if (static_cast<size_t>(q.size()) != list.size())
			return false;

		for(int i = 0; i < q.size(); i++)
		{
			if (q.getAtIndex(i) != list.begin()[i])
				return false;
		}

		return true;
	}

} // anonymous namespace

void runQueueTests()
{
	CZ_LOG(logDefault, Log, F("Running QueueTests..."));

	TStaticFixedCapacityQueue<int, 5> q;

	printQueue(q);
	CZ_TEST(q.push(0));
	CZ_TEST(q.push(1));
	CZ_TEST(q.push(2));
	CZ_TEST(q.push(3));
	CZ_TEST(q.push(4));

	CZ_TEST(equals(q, { 0,1,2,3,4 }));
	printQueue(q);
	CZ_TEST(q.push(5) == false);
	printQueue(q);

	CZ_TEST(q.pop() == 0);
	CZ_TEST(q.pop() == 1);
	printQueue(q);
	CZ_TEST(equals(q, { 2,3,4 }));

	CZ_TEST(q.push(5));
	printQueue(q);
	CZ_TEST(equals(q, { 2,3,4,5 }));

	CZ_TEST(q.push(2));
	CZ_TEST(equals(q, { 2,3,4,5,2 }));
	CZ_TEST(q.remove(2) == 2);
	CZ_TEST(equals(q, { 3,4,5 }));

	CZ_TEST(q.front() == 3);
	CZ_TEST(q.back() == 5);

	q.clear();
	CZ_TEST(q.size() == 0);

	CZ_LOG(logDefault, Log, F("Finished QueueTests"));
}

} // namespace cz
