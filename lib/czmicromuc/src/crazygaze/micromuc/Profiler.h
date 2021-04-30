#pragma once

#include "crazygaze/micromuc/czmicromuc.h"
#include <Arduino.h>

namespace cz
{

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


struct Profiler
{
	struct Section
	{
		const __FlashStringHelper* name;
		unsigned long totalMicros;
		float totalSeconds;
		unsigned long count;
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

}; // Profiler

} // namespace cz

#define PROFILE_SCOPE(name) \
	static cz::Profiler::Section CONCATENATE(PROFILE_SECTION_, __LINE__)(name); \
	cz::Profiler::Scope CONCATENATE(PROFILE_SCOPE_, __LINE__)(CONCATENATE(PROFILE_SECTION_, __LINE__));

#define CREATE_PROFILER(capacity) \
	cz::Profiler::Point gProfilerPoints[capacity]; \
	cz::Profiler gProfiler(gProfilerPoints, capacity);

