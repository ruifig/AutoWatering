#include "Profiler.h"
#include "Logging.h"
#include "StringUtils.h"

#if CZ_PROFILER
extern cz::Profiler gProfiler;

namespace cz
{

Profiler::Section::Section(const __FlashStringHelper* name)
{
	this->name = name;

	if (gProfiler.lastSection)
	{
		gProfiler.lastSection->next = this;
	}
	else
	{
		gProfiler.rootSection = this;
	}
	gProfiler.lastSection = this;
}

Profiler::Scope::Scope(Section& section)
{
	if (gProfiler.pointsCount >= gProfiler.pointsCapacity - 1)
	{
		point = nullptr;
		return;
	}

	point = &gProfiler.points[gProfiler.pointsCount];
	gProfiler.pointsCount++;

	point->duration = micros();
	point->section = &section;
	point->level = gProfiler.level;
	gProfiler.level++;
}

Profiler::Scope::~Scope()
{
	if (point)
	{
		point->duration = micros() - point->duration;
		point->section->totalMicros += point->duration;
		point->section->count++;
		gProfiler.level--;
	}
}

Profiler::Profiler(Point* buffer, int capacity)
{
	points = buffer;
	pointsCapacity = capacity;
	startRun();
}

void Profiler::startRun()
{
	points[0] = {0,0};
	pointsCount = 0;
	level = 0;
}

void Profiler::log()
{
	LogOutput::logToAllSimple(F("** Profiler **\n"));

	{
		Section* section = rootSection;
		LogOutput::logToAllSimple(F("  Sections\n"));
		while(section)
		{
			LogOutput::logToAllSimple(F("    "));
			LogOutput::logToAllSimple(section->name);
			LogOutput::logToAllSimple(
				formatString(F(": calls=%lu, time %lu microseconds (%lu milliseconds, %lu seconds)\n"),
					section->count,
					section->totalMicros,
					section->totalMicros / 1000,
					section->totalMicros / 1000000
					));
			
			section = section->next;
		}
	}

	{
		Point* p = points;
		LogOutput::logToAllSimple(F("  Points\n"));
		while(p->section)
		{
			char indent[50];
			LogOutput::logToAllSimple(formatString(F("    %s"), duplicateChar(indent, p->level, ' ')));
			LogOutput::logToAllSimple(p->section->name);
			LogOutput::logToAllSimple(
				formatString(F(": time %lu microseconds (%lu milliseconds, %lu seconds)\n"),
					p->duration,
					p->duration / 1000,
					p->duration / 1000000
					));
			
			p++;
		}
		LogOutput::logToAllSimple(F("** Done **\n"));
	}

	Serial.flush();
}


} // namespace cz


#endif
