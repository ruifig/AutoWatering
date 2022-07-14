#pragma once

#include "crazygaze/micromuc/Queue.h"
#include "crazygaze/micromuc/Array.h"

#include "crazygaze/micromuc/Logging.h"
#include "crazygaze/micromuc/StringUtils.h"

namespace cz
{

/*
 * Not sure what to call this.
 * It's to implement a queue system for the sensors and motors, where we can specify how many we allow to be active at one given time
 */
template<typename T, int NumSlots, int MaxActive>
class TSemaphoreQueue
{
  public:
	using Type = T;
	using QueueType = TSemaphoreQueue<T, NumSlots, MaxActive>;

	class Handle
	{
	  public:

	  	enum State : uint8_t
		{
			Inactive,
			Queued, // It's registered in the queue so a call to acquire will eventually succeed as new active slots are released by other handles
			Active // Currently holds an active slot
		};

		Handle& operator=(const Handle&) = delete;
		Handle(const Handle&) = delete;

		Handle()
		{
		}

	  	Handle(QueueType& q, const Type& obj)
			: m_q(&q)
			, m_obj(obj)
		{
		}

		Handle(Handle&& other)
		{
			moveFrom(std::move(other));
		}

		Handle& operator=(Handle&& other)
		{
			if (this != &other)
			{
				moveFrom(std::move(other));
			}
			return *this;
		}

		void moveFrom(Handle&& other)
		{
			release();
			m_q = other.m_q;
			m_obj = other.m_obj;
			m_state = other.m_state;
			other.m_q = nullptr;
			other.m_state = State::Inactive;
		}

		~Handle()
		{
			release();
		}

		bool isActive() const
		{
			return m_state == State::Active;
		}

		bool isActiveOrQueued() const
		{
			return m_state==State::Queued || m_state==State::Active ? true : false;
		}

		bool tryAcquire(bool registerInterest)
		{
			if (m_q == nullptr)
			{
				return false;
			}

			if (m_q->tryAcquire(m_obj, registerInterest))
			{
				m_state = State::Active;
				return true;
			}
			else if (registerInterest)
			{
				CZ_ASSERT(m_state==State::Inactive || m_state==State::Queued);
				// We might be already in 
				m_state = State::Queued;
			}

			return false;
		}

		void release()
		{
			if (m_q && m_state != State::Inactive)
			{
				m_q->release(m_obj);
				m_state = State::Inactive;
			}
		}

	  protected:
		QueueType* m_q = nullptr;
		Type m_obj;
		State m_state = State::Inactive;
	};


	Handle createHandle(Type& obj)
	{
		return Handle(*this, obj);
	}

	/**
	 * Tries to acquire an active slot for the specified object.
	 * If acquisition fails, it can optionally add the object to an interest queue, so that a future tryAcquire success is guarenteed as
	 * slots are release by other objects
	 * 
	 * \param obj Object to register as acquiring a slot
	 * \param registerInterest
	 * 		If this is true and we fail to acquire a slot, it adds the object to the request queue. As slots are acquired and released.
	 * 		If making calls for a give object with this set to true, calls to "release" must be made even if a slot is not acquired, in
	 * 		order to remove the object from the interest queue
	 * \return
	 * 		True if an active slot is acquired, either with this call, or due to a previous call.
	 */
	bool tryAcquire(const Type& obj, bool registerInterest)
	{
		//CZ_LOG(logDefault, Log, F("%s(%d,%s)"), __FUNCTION__, (int)obj, registerInterest ? "true" : "false");

		debugLog("BEFORE");

		// Object is already active, so do nothing
		if (m_active.find(obj))
		{
			//CZ_LOG(logDefault, Log, F("    Object is already active"));
			//CZ_LOG(logDefault, Log, F("    Return TRUE"));
			return true;
		}

		// If no available active slots, nothing we can do other than register interest (if requested)
		if (m_active.size() == m_active.capacity())
		{
			//CZ_LOG(logDefault, Log, F("    No active slots available"));
			if (registerInterest && !m_q.find(obj))
			{
				//CZ_LOG(logDefault, Log, F("    Registering interest"));
				m_q.push(obj);
			}
			//CZ_LOG(logDefault, Log, F("    Return FALSE"));
			debugLog("AFTER");
			return false;
		}

		// If there active slots available:
		// * If the queue is empty, we can acquire
		// * If the queue is not empty, we check if the object is the next in queue
		// * Register interest if we failed to acquire and object NOT in queue already
		Type next;
		if ( m_q.isEmpty())
		{
			//CZ_LOG(logDefault, Log, F("    Queue empty. Assigning a slot"));
			m_active.push(obj);
			//CZ_LOG(logDefault, Log, F("    Return TRUE"));
			debugLog("AFTER");
			return true;
		}
		else if (m_q.peek(next) && next==obj)
		{
			//CZ_LOG(logDefault, Log, F("    Object at the front of the queue. Assigning slot"));
			m_q.pop();
			m_active.push(obj);
			//CZ_LOG(logDefault, Log, F("    Return TRUE"));
			debugLog("AFTER");
			return true;
		}
		else if (registerInterest && !m_q.find(obj))
		{
			//CZ_LOG(logDefault, Log, F("    Registering interest"));
			m_q.push(obj);
		}

		//CZ_LOG(logDefault, Log, F("    Return FALSE"));
		debugLog("AFTER");
		return false;
	}

	/**
	 * Releases hold of an active slot
	 * \return
	 * 		True if the object was holding to an active slot
	 * 		False if the object was not holding to an active slot
	 * 
	 * \note
	 * 	Calling this with an "obj" that didn't manage to acquire an active slot causes no problems 
	 */
	bool release(const Type& obj)
	{
		//CZ_LOG(logDefault, Log,  F("%s(%s)"), __FUNCTION__, (int)obj);
		// Remove any interest from acquiring a slot
		m_q.remove(obj);
		// Release the active slot this object is taking (if any)
		int count = m_active.removeIfExists(obj);
		//CZ_LOG(logDefault, Log,  F("    Return %s"), count==0 ? "false" : "true");
		return count==0 ? false : true;
	}

  private:

	void debugLog(const char* title)
	{
		return;
		char buf[2048];
		buf[0]=0;
		strCatPrintf(buf, F("    %s-m_q[%d]={"), title, m_q.size());
		for(int i=0; i<m_q.size(); i++)
		{
			strCatPrintf(buf, F("%d,"), (int)m_q.getAtIndex(i));
		}

		strCatPrintf(buf, F("}, m_active[%d]={"), m_active.size());
		for(auto&& e : m_active)
		{
			strCatPrintf(buf, F("%d,"), (int)e);
		}
		strCatPrintf(buf, F("}"));
		CZ_LOG(logDefault, Log, F("%s"), buf);
	};

	static_assert(NumSlots>=MaxActive, "MaxActive needs to be <= NumSlots");
	TStaticFixedCapacityQueue<Type, NumSlots> m_q;
	TStaticArray<Type, MaxActive, true> m_active;

};


} // namespace cz
