#pragma once

#include <utility>

namespace cz
{
namespace TickerPolicy
{

/**
 * Ticking will be based on time elapsed since the last tick.
 * obj->tick should return how many seconds to wait until the next tick.
 */
template <class T>
struct TTime
{
  public:
	using TimeType = T;

	TimeType getCountdown() const
	{
		return m_countdown;
	}

  protected:
	TTime()
	{
		m_countdown = 0;
	}

	bool update(TimeType deltatime)
	{
		m_countdown -= deltatime;
		return (m_countdown <= 0 ? true : false);
	}

	void reset(TimeType countdown = 0)
	{
		m_countdown = countdown;
	}

	// Countdown to next tick
	TimeType m_countdown;
};

/**
 * Similar to TTime, but ignores the countdown, so it causes a tick to happen every call.
 * Useful for debugging.
 */
template <class T>
struct TTimeAlwaysTick
{
  public:
	using TimeType = T;

	TimeType getCountdown() const
	{
		return 0;
	}

  protected:
	TTimeAlwaysTick()
	{
	}

	bool update(TimeType deltatime)
	{
		return true;
	}

	void reset(TimeType countdown = 0)
	{
	}

};

};  // namespace TickerPolicy

template <class TObjectType, class TTimeType, class TTickingMethod = TickerPolicy::TTime<TTimeType> >
class TTicker : public TTickingMethod
{
  public:
	using ObjectType = TObjectType;
	using TimeType = TTimeType;

	TTicker(bool tickEnabled = true)
	{
		m_tickEnabled = tickEnabled;
		TTickingMethod::reset();
	}

	template <class... Args>
	TTicker(bool tickEnabled, Args&&... args) : m_obj(std::forward<Args>(args)...), m_tickEnabled(tickEnabled)
	{
		TTickingMethod::reset();
	}

	~TTicker() {}

	TimeType tick(TimeType deltatime)
	{
		if (m_tickEnabled)
		{
			if (TTickingMethod::update(deltatime))
			{
				auto res = m_obj.tick(m_timeSinceLastTick + deltatime);
				if (res == 0)
					m_tickEnabled = false;
				TTickingMethod::reset(res);
				m_timeSinceLastTick = 0;
			}
			else
			{
				m_timeSinceLastTick += deltatime;
			}
		}

		return TTickingMethod::getCountdown();
	}

	ObjectType& getObj() { return m_obj; }
	const ObjectType& getObj() const { return m_obj; }
	
	// Not allowing these operators, because it would be too easy to mess it up by calling
	// the object's tick function directly, since both TTicker and ObjectType has a tick function
#if 0
	ObjectType* operator->() { return &m_obj; }
	const ObjectType* operator->() const { return &m_obj; }
#endif

	void start(TimeType countdown)
	{
		m_timeSinceLastTick = 0;
		m_tickEnabled = true;
		TTickingMethod::reset(countdown);
	}

	void stop() { m_tickEnabled = false; }

	bool isEnabled() const { return m_tickEnabled; }

  private:
	ObjectType m_obj;
	// Accumulates the time since the last tick
	TimeType m_timeSinceLastTick = 0;
	// int m_countdown;
	bool m_tickEnabled = false;
};

struct FunctionTickerObj
{
  public:
	using FunctionType = void (*)();

	/*!
	 * @param func Function to call
	 * @param interval in seconds to call the function
	 */
	FunctionTickerObj(FunctionType func_, float interval_) : func(func_), interval(interval_) {}

	FunctionTickerObj() = default;
	FunctionTickerObj(const FunctionTickerObj&) = default;

	// So it works with Ticker
	FunctionTickerObj* operator->() { return this; }

	float tick(float /* deltaSeconds */)
	{
		func();
		return interval;
	}

	FunctionType func;
	float interval = 0;
};

struct FunctionTicker
{
  public:
	FunctionTicker(FunctionTickerObj::FunctionType func_, float interval_) : ticker(true, func_, interval_) {}

	void tick(float deltaSeconds) { ticker.tick(deltaSeconds); }

	void setInterval(float deltaSeconds)
	{
		ticker.getObj().interval = deltaSeconds;
		ticker.start(deltaSeconds);
	}

	void stop() { ticker.stop(); }

  protected:
	TTicker<FunctionTickerObj, float> ticker;
};

}  // namespace cz