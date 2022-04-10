#pragma once

#include "crazygaze/micromuc/czmicromuc.h"
#include <Arduino.h>

#define CONCATENATE_IMPL(s1,s2) s1##s2
#define CONCATENATE(s1,s2) CONCATENATE_IMPL(s1,s2)

// Note: __COUNTER__ Expands to an integer starting with 0 and incrementing by 1 every time it is used in a source file or included headers of the source file.
#ifdef __COUNTER__
	#define ANONYMOUS_VARIABLE(str) \
		CONCATENATE(str,__COUNTER__)
#else
	#define ANONYMOUS_VARIABLE(str) \
		CONCATENATE(str,__LINE__)
#endif

#if CZ_PROFILER

namespace cz
{

struct Profiler
{
	struct Section
	{
		const __FlashStringHelper* name;
		unsigned long totalMicros;
		float totalSeconds;
		unsigned long count;
		unsigned long longestDurationMicros;
		unsigned long shortestDurationMicros;
		Section* next;
		Section(const __FlashStringHelper* name);
	};

	struct Point
	{
		unsigned long duration;
		Section* section;
		uint8_t level;
	};

	struct Scope
	{
		Scope(Section& section);
		~Scope();
		Point* point;
	};

	Section* rootSection;
	Section* lastSection;
	Point* points;
	uint8_t level;
	int pointsCapacity;
	int pointsCount;

	Profiler(Point* buffer, int capacity);

	void startRun();
	void log();
	void reset();

}; // Profiler

} // namespace cz

	#define PROFILE_SCOPE(name) \
		static cz::Profiler::Section CONCATENATE(PROFILE_SECTION_, __LINE__)(name); \
		cz::Profiler::Scope CONCATENATE(PROFILE_SCOPE_, __LINE__)(CONCATENATE(PROFILE_SECTION_, __LINE__));

	#define PROFILER_CREATE(capacity) \
		cz::Profiler::Point gProfilerPoints[capacity]; \
		cz::Profiler gProfiler(gProfilerPoints, capacity);

	#define PROFILER_STARTRUN() gProfiler.startRun()
	#define PROFILER_LOG() gProfiler.log()
	#define PROFILER_RESET() gProfiler.reset()

#else // CZ_PROFILER

	#define PROFILE_SCOPE(name) 
	#define PROFILER_CREATE(capacity)
	#define PROFILER_STARTRUN()
	#define PROFILER_LOG()
	#define PROFILER_RESET()
#endif





