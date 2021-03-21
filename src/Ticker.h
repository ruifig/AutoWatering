#pragma once

namespace cz
{

namespace TickerPolicy
{

	/**
	* Ticking will be based on time elapsed since the last tick.
    * obj->tick should return how many seconds to wait until the next tick.
	*/
	template<class T>
	struct Time
	{
	public:
		typedef T TimeType; 
		TimeType getCountdown() const
		{
			return m_countdown;
		}
	protected:
		Time()
		{
			m_countdown = 0;
		}

		bool update(TimeType deltatime)
		{
			m_countdown -= deltatime;
			return (m_countdown<=0 ? true : false);
		}

		void reset(TimeType countdown=0)
		{
			m_countdown = countdown;
		}

	private:
		// Countdown to next tick
		TimeType m_countdown;
	};

}; // namespace TickerPolicy

template<class OBJECTTYPE, class TIMETYPE, class TICKINGMETHOD=TickerPolicy::Time<TIMETYPE> >
class Ticker : public TICKINGMETHOD
{
public:
	typedef OBJECTTYPE ObjectType;
	typedef TIMETYPE TimeType;

private:

	template<class T>
	void initializeObj(T* obj)
	{
		// Does nothing, since we have an object
	}
	template<class T>
	void initializeObj(T** obj)
	{
		*obj = nullptr;
	}
public:

	Ticker()
	{
		initializeObj(&m_obj);
	}
	explicit Ticker(ObjectType _tickedObj, bool tickEnabled=true) :
		m_obj(std::move(_tickedObj)), 
		m_tickEnabled(tickEnabled)
	{
		reset();
	}

	~Ticker()
	{
	}

	void tick(TimeType deltatime)
	{
		if (m_tickEnabled)
		{
			if (update(deltatime))
			{
				auto res = m_obj->tick(m_timeSinceLastTick + deltatime);
				if (res==0)
					m_tickEnabled = false;
				reset(res);
				m_timeSinceLastTick = 0;
			}
			else
			{
				m_timeSinceLastTick += deltatime;
			}
		}
	}

	/*
	Same thing as tick, but with an extra parameter
	*/
	template<class ExtraParameter1>
	void tickExt1(TimeType deltatime, ExtraParameter1& extra)
	{
		if (m_tickEnabled)
		{
			if (update(deltatime))
			{
				auto res = m_obj->tick(m_timeSinceLastTick + deltatime, extra);
				if (res==0)
					m_tickEnabled = false;
				reset(res);
				m_timeSinceLastTick = 0;
			}
			else
			{
				m_timeSinceLastTick += deltatime;
			}
		}
	}

	template<class ExtraParameter1, class ExtraParameter2>
	void tickExt2(TimeType deltatime, ExtraParameter1& extra1, ExtraParameter2& extra2)
	{
		if (m_tickEnabled)
		{
			if (update(deltatime))
			{
				TimeType res = m_obj->tick(m_timeSinceLastTick + deltatime, extra1, extra2);
				if (res==0)
					m_tickEnabled = false;
				reset(res);
				m_timeSinceLastTick = 0;
			}
			else
			{
				m_timeSinceLastTick += deltatime;
			}
		}
	}

	template<class ExtraParameter1, class ExtraParameter2, class ExtraParameter3>
	void tickExt2(TimeType deltatime, ExtraParameter1& extra1, ExtraParameter2& extra2, ExtraParameter3& extra3)
	{
		if (m_tickEnabled)
		{
			if (update(deltatime))
			{
				TimeType res = m_obj->tick(m_timeSinceLastTick + deltatime, extra1, extra2, extra3);
				if (res==0)
					m_tickEnabled = false;
				reset(res);
				m_timeSinceLastTick = 0;
			}
			else
			{
				m_timeSinceLastTick += deltatime;
			}
		}
	}

	ObjectType& getObj()
	{
		return m_obj;
	}

	const ObjectType& getObj() const
	{
		return m_obj;
	}

	void setObj(const ObjectType& obj, bool needsticking)
	{
		m_obj = obj;
		m_timeSinceLastTick=0;
		m_tickEnabled = needsticking;
		reset();
	}

	void setObj(ObjectType&& obj, bool needsticking)
	{
		m_obj = std::move(obj);
		m_timeSinceLastTick=0;
		m_tickEnabled = needsticking;
		reset();
	}

	void start(TimeType countdown)
	{
		m_timeSinceLastTick=0;
		m_tickEnabled = true;
		reset(countdown);
	}

	void stop()
	{
		m_tickEnabled = false;
	}

	bool isEnabled() const
	{
		return m_tickEnabled;
	}

private:
	ObjectType m_obj;
	// Accumulates the time since the last tick
	TimeType m_timeSinceLastTick=0;
	//int m_countdown;
	bool m_tickEnabled=false;
};


struct FunctionTickerObj
{
public:
    using FunctionType = void (*)();

	/*!
	 * \param interval
	 *	Tick interval, in seconds
	 */
	FunctionTickerObj(float interval_, FunctionType func_) : interval(interval_), func(func_)
	{
	}
	FunctionTickerObj() = default;
	FunctionTickerObj(const FunctionTickerObj&) = default;

	// So it works with Ticker
	FunctionTickerObj* operator->() { return this; }

	float tick(float /* deltaSeconds */)
	{
		func();
		return interval;
	}

	float interval=0;
	FunctionType func;
};

struct FunctionTicker
{
public:
	FunctionTicker() : ticker(FunctionTickerObj(), false)
	{
	}
	FunctionTicker(float interval_, FunctionTickerObj::FunctionType func_)
		: ticker(FunctionTickerObj(interval_, func_), true)
	{
	}

	void tick(float deltaSeconds)
	{
		ticker.tick(deltaSeconds);
	}

	void setInterval(float deltaSeconds)
	{
		ticker.getObj().interval = deltaSeconds;
		ticker.start(deltaSeconds);
	}

	void stop()
	{
		ticker.stop();
	}
protected:
	Ticker<FunctionTickerObj, float> ticker;
};

} // namespace cz
