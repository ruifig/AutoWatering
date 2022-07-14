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
template<typename IDType, int NumSlots, int MaxActive>
class TSemaphoreQueue
{
  public:
	using QueueType = TSemaphoreQueue<IDType, NumSlots, MaxActive>;

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

			if (m_q->tryAcquire(m_id, registerInterest))
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
				m_q->release(m_id);
				m_state = State::Inactive;
			}
		}

	  protected:

	  	friend QueueType;

	  	Handle(QueueType& q, const IDType& id)
			: m_q(&q)
			, m_id(id)
		{
		}

		void moveFrom(Handle&& other)
		{
			release();
			m_q = other.m_q;
			m_id = other.m_id;
			m_state = other.m_state;
			other.m_q = nullptr;
			other.m_state = State::Inactive;
		}

		QueueType* m_q = nullptr;
		IDType m_id;
		State m_state = State::Inactive;
	};

	Handle createHandle()
	{
		return Handle(*this, m_idCounter++);
	}


  protected:

	/**
	 * Tries to acquire an active slot for the specified id.
	 * If acquisition fails, it can optionally add the id to an interest queue, so that a future tryAcquire success is guarenteed as
	 * slots are release by other ids
	 * 
	 * \param id id to register as acquiring a slot
	 * \param registerInterest
	 * 		If this is true and we fail to acquire a slot, it adds the id to the request queue. As slots are acquired and released.
	 * 		If making calls for a give id with this set to true, calls to "release" must be made even if a slot is not acquired, in
	 * 		order to remove the id from the interest queue
	 * \return
	 * 		True if an active slot is acquired, either with this call, or due to a previous call.
	 */
	bool tryAcquire(const IDType& id, bool registerInterest)
	{
		// id is already active, so do nothing
		if (m_active.find(id))
		{
			return true;
		}

		// If no available active slots, nothing we can do other than register interest (if requested)
		if (m_active.size() == m_active.capacity())
		{
			if (registerInterest && !m_q.find(id))
			{
				m_q.push(id);
			}
			return false;
		}

		// If there active slots available:
		// * If the queue is empty, we can acquire
		// * If the queue is not empty, we check if the id is the next in queue
		// * Register interest if we failed to acquire and id NOT in queue already
		IDType next;
		if ( m_q.isEmpty())
		{
			m_active.push(id);
			return true;
		}
		else if (m_q.peek(next) && next==id)
		{
			m_q.pop();
			m_active.push(id);
			return true;
		}
		else if (registerInterest && !m_q.find(id))
		{
			m_q.push(id);
		}

		return false;
	}

	/**
	 * Releases hold of an active slot
	 * \return
	 * 		True if the id was holding to an active slot
	 * 		False if the id was not holding to an active slot
	 * 
	 * \note
	 * 	Calling this with an "id" that didn't manage to acquire an active slot causes no problems 
	 */
	bool release(const IDType& id)
	{
		// Remove any interest from acquiring a slot
		m_q.remove(id);
		// Release the active slot this id is taking (if any)
		int count = m_active.removeIfExists(id);
		return count==0 ? false : true;
	}

  private:

	static_assert(NumSlots>=MaxActive, "MaxActive needs to be <= NumSlots");
	IDType m_idCounter = 0;
	TStaticFixedCapacityQueue<IDType, NumSlots> m_q;
	TStaticArray<IDType, MaxActive, true> m_active;

};


} // namespace cz
