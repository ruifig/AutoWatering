/********************************************************************
	CrazyGaze (http://www.crazygaze.com)
	Author : Rui Figueira
	Email  : rui@crazygaze.com
	
	purpose:
	
*********************************************************************/

#include "czMicroMuc.h"
#include "Logging.h"
#include <string.h>

#define LOG_TIME 1
#define LOG_CATEGORY 1
#define LOG_VERBOSITY 1

CZ_DEFINE_LOG_CATEGORY(logDefault)

namespace cz
{

#if !CZ_LOG_ENABLED
LogCategoryLogNone logNone;
#endif

LogCategoryBase::LogCategoryBase(const char* name, LogVerbosity verbosity, LogVerbosity compileTimeVerbosity) : m_name(name)
, m_verbosity(verbosity)
, m_compileTimeVerbosity(compileTimeVerbosity)
{
	if (ms_first==nullptr)
	{
		ms_first = this;
	}
	else
	{
		// Find the last category
		LogCategoryBase* ptr = ms_first;
		while(ptr->m_next)
		{
			ptr = ptr->m_next;
		}
		ptr->m_next = this;
	}
}

const char* logVerbosityToString(LogVerbosity v);
{
	static strs[6] =
	{
		"NNN",
		"FTL",
		"ERR",
		"WRN",
		"LOG",
		"VER"
	};

	return strs[v];
}

void LogCategoryBase::setVerbosity(LogVerbosity verbosity)
{
	// Take into considering the minimum compiled verbosity
	m_verbosity = LogVerbosity( std::min((int)m_compileTimeVerbosity, (int)verbosity) );
}

cz::LogCategoryBase* LogCategoryBase::getNext()
{
	return m_next;
}

cz::LogCategoryBase* LogCategoryBase::getFirst()
{
	return ms_first;
}

cz::LogCategoryBase* LogCategoryBase::find(const char* name)
{
	LogCategoryBase* ptr = ms_first;
	while(ptr)
	{
		if (strcmp(ptr->m_name,name) == 0)
			return ptr;
		ptr = ptr->m_next;
	};

	return nullptr;
}

LogOutput::SharedData* LogOutput::getSharedData()
{
	// This is thread safe, according to C++11 (aka: Magic Statics)
	static SharedData data;
	return &data;
}

LogOutput::LogOutput()
{
	auto data = getSharedData();
	auto lk = std::unique_lock<std::mutex>(data->mtx);
	data->outputs.push_back(this);
}

LogOutput::~LogOutput()
{
	auto data = getSharedData();
	auto lk = std::unique_lock<std::mutex>(data->mtx);
	data->outputs.erase(std::find(data->outputs.begin(), data->outputs.end(), this));
}

void LogOutput::logToAll(const LogCategoryBase* category, LogVerbosity verbosity, const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	#if LOG_TIME
		// Example of max length: 50:24:60:60:999 \n
		#define TIME_BUF_SIZE 17
		char timeStr[TIME_BUF_SIZE];

		unsigned long ms = millis();

		int days = ms / 86400000;
		ms = ms - (days * 86400000 );

		int hours = ms / 3600000;
		ms = ms - (hours * 3600000);

		int minutes = ms / 60000;
		ms = ms - (minutes * 60000);

		int seconds = ms / 1000;
		ms = ms - (seconds * 1000);

		snprintf(timeStr, TIME_BUF_SIZE, "%02d:%02d:%02d:%02d:%03d ", days, hours, minutes, seconds, ms);
	#else
		const char* timeStr = "";
	#endif


	#if LOG_CATEGORY
		// Example of long category name: logReallyBigLongCategory: \n
		#define CATEGORY_BUF_SIZE 27
		char categoryNameStr[CATEGORY_BUF_SIZE];
		snprintf(categoryNameStr, CATEGORY_BUF_SIZE, "%s: ", category->getName());
	#else
		const char* categoryNameStr = "";
	#endif

	#if LOG_VERBOSITY
		// Example of max size: WRN: \n
		#define VERBOSITY_BUF_SIZE 6
		char verbosityStr[VERBOSITY_BUF_SIZE];
		snprintf(verbosityStr, VERBOSITY_BUF_SIZE, "%s: ", logVerbosityToString(verbosity));
	#else
		const char* verbosityStr = "";
	#endif

	// Log prefix
	{
		const char* str = formatStringVA("%s%s%s%s", timeStr, categoryNameStr, verbosityStr);
		logToAllSimple(verbosity, str);
	}

	// Log message
	{
		const char* str = formatStringVA(fmt, args);
		logToAllSimple(verbosity, str);
		logToAllSimple(verbosity, "\r\n");
	}

	va_end(args);

}

void LogOutput::logToAllSimple(LogVerbosity verbosity, const char* str);
{
	auto data = getSharedData();
	auto lk = std::unique_lock<std::mutex>(data->mtx);
	for (auto&& out : data->outputs)
	{
		out->logSimple(verbosity, str);
	}
}


} // namespace cz

