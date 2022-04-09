/********************************************************************
	CrazyGaze (http://www.crazygaze.com)
	Author : Rui Figueira
	Email  : rui@crazygaze.com
	
	purpose:
	Logging functions/classes, somewhat inspired by how UE4 does it
	
*********************************************************************/

#pragma once

#include "crazygaze/micromuc/czmicromuc.h"
#include "crazygaze/micromuc/Array.h"
#include <Arduino.h>

#if _GLIBCXX_HAS_GTHREADS
	#include <mutex>
#else
	namespace std
	{
		/*
		* Mock mutex.
		* I'll implement it at some later point if it's needed
		*/
		class mutex
		{
		};

		/*
		* Mock unique_lock.
		* I'll implement it at some later point if it's needed
		*/
		template<typename T>
		class unique_lock
		{
		public:
			unique_lock(T mtx)
			{
				ms_dummy++;
			}

			~unique_lock()
			{
				ms_dummy--;
			}
		private:
			static inline int ms_dummy;
		};
	} // namespace std
#endif


namespace cz
{

enum class LogVerbosity
{
	None, // Used internally only
	Fatal,
	Error,
	Warning,
	Log,
	Verbose
};

const char* logVerbosityToString(LogVerbosity v);

// Globally sets maximum compile time verbosity
#define CZ_LOG_MAXIMUM_VERBOSITY Verbose

class LogCategoryBase
{
  public:
	LogCategoryBase(const char* name, LogVerbosity verbosity, LogVerbosity compileTimeVerbosity);
	const char* getName() const
	{
		return m_name;
	}
	bool isSuppressed(LogVerbosity verbosity) const
	{
		return verbosity > m_verbosity;
	}
	void setVerbosity(LogVerbosity verbosity);
	LogCategoryBase* getNext();
	static LogCategoryBase* getFirst();
	static LogCategoryBase* find(const char* name);

  protected:
	const char* m_name;
	LogVerbosity m_verbosity;
	LogVerbosity m_compileTimeVerbosity;
	LogCategoryBase* m_next = nullptr;
	static LogCategoryBase* ms_first;
};

template<LogVerbosity DEFAULTVERBOSITY, LogVerbosity COMPILETIMEVERBOSITY>
class LogCategory : public LogCategoryBase
{
public:
	LogCategory(const char* name) : LogCategoryBase(name, DEFAULTVERBOSITY, COMPILETIMEVERBOSITY)
	{
	}

	// Compile time verbosity
	enum
	{
		CompileTimeVerbosity  = (int)COMPILETIMEVERBOSITY
	};

private:
};

class LogOutput
{
public:
	LogOutput();
	virtual ~LogOutput();

	/**
	 * Logs a message, with all the information that is enabled (time, category, verbosity), and moves to the next line
	 **/
	static void logToAll(const LogCategoryBase* category, LogVerbosity verbosity, const char* fmt, ...) __attribute__ ((format (printf, 3,4)));
	static void logToAll(const LogCategoryBase* category, LogVerbosity verbosity, const __FlashStringHelper* fmt, ...);

	/**
	 * Logs a message without any extra information. Also, it doesn't add a new line at the end
	 **/
	static void logToAllSimple(const char* str);
	static void logToAllSimple(const __FlashStringHelper* str);

	static void flush();

private:
	
	virtual void logSimple(const char* str) = 0;
	virtual void logSimple(const __FlashStringHelper* str) = 0;
	virtual void flushImpl() = 0;
	static void logPrefix(const LogCategoryBase* category, LogVerbosity verbosity);

	struct SharedData
	{
		std::mutex mtx;
		cz::TStaticArray<LogOutput*, 4, true> outputs;
		bool logToDebugger = true;
	};
	static SharedData* getSharedData();
};

#if CZ_SERIAL_LOG_ENABLED
class SerialLogOutput : public LogOutput
{
public:
	// Initializes using the default Serial object
	void begin(unsigned long speed = 115200);
	// Initializes using any stream
	void begin(arduino::HardwareSerial& stream, unsigned long speed = 115200);
private:
	virtual void logSimple(const char* str) override;
	virtual void logSimple(const __FlashStringHelper* str) override;
	virtual void flushImpl() override;
	arduino::HardwareSerial* m_stream;
};
#endif


#if CZ_LOG_ENABLED

	#define CZ_DECLARE_LOG_CATEGORY(NAME, DEFAULT_VERBOSITY, COMPILETIME_VERBOSITY) \
		extern class LogCategory##NAME : public ::cz::LogCategory<::cz::LogVerbosity::DEFAULT_VERBOSITY, ::cz::LogVerbosity::COMPILETIME_VERBOSITY> \
		{ \
			public: \
			LogCategory##NAME() : LogCategory(#NAME) {} \
		} NAME;

	#define CZ_DEFINE_LOG_CATEGORY(NAME) LogCategory##NAME NAME;

	#define CZ_LOG_CHECK_COMPILETIME_VERBOSITY(NAME, VERBOSITY) \
		(((int)::cz::LogVerbosity::VERBOSITY <= LogCategory##NAME::CompileTimeVerbosity) && \
		 ((int)::cz::LogVerbosity::VERBOSITY <= (int)::cz::LogVerbosity::CZ_LOG_MAXIMUM_VERBOSITY))

	#define CZ_LOG(NAME, VERBOSITY, fmt, ...)                                                                \
		{                                                                                                    \
			if (CZ_LOG_CHECK_COMPILETIME_VERBOSITY(NAME, VERBOSITY))                                         \
			{                                                                                                \
				if (!NAME.isSuppressed(::cz::LogVerbosity::VERBOSITY))                                       \
				{                                                                                            \
					::cz::LogOutput::logToAll(&NAME, ::cz::LogVerbosity::VERBOSITY, fmt, ##__VA_ARGS__);     \
					if (::cz::LogVerbosity::VERBOSITY == ::cz::LogVerbosity::Fatal)                          \
					{                                                                                        \
						::cz::_doAssert(__FILENAME__, __LINE__, fmt, ##__VA_ARGS__);                         \
					}                                                                                        \
				}                                                                                            \
			}                                                                                                \
		}


#else


	struct LogCategoryLogNone : public LogCategory<LogVerbosity::None, LogVerbosity::None>
	{
		LogCategoryLogNone() : LogCategory("LogNone") {};
		void setVerbosity(LogVerbosity verbosity) {}
	};
	extern LogCategoryLogNone logNone;

	#define CZ_DECLARE_LOG_CATEGORY(NAME, DEFAULT_VERBOSITY, COMPILETIME_VERBOSITY) extern ::cz::LogCategoryLogNone& NAME;
	#define CZ_DEFINE_LOG_CATEGORY(NAME) ::cz::LogCategoryLogNone& NAME = ::cz::logNone;
	#define CZ_LOG(NAME, VERBOSITY, fmt, ...)                               \
		{                                                                   \
			if (::cz::LogVerbosity::VERBOSITY == ::cz::LogVerbosity::Fatal) \
			{                                                               \
				::cz::_doAssert(__FILENAME__, __LINE__, fmt, ## __VA_ARGS__);    \
			}                                                               \
		}


#endif

} // namespace cz

CZ_DECLARE_LOG_CATEGORY(logDefault, Log, Verbose)

