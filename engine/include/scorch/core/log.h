#pragma once
#include <cstdio>
#include <string>
#include <cassert>

template< typename... Args >
std::string string_format(const char* format, Args... args) {
	int length = std::snprintf(nullptr, 0, format, args...);
	assert(length >= 0);

	std::string str(length, 1);
	std::snprintf(str.data(), (size_t)length + 1, format, args...);

	return str;
}


namespace SC
{
	enum class LogSeverity
	{
		LogTrace,
		LogInfo,
		LogWarning,
		LogError,
		LogFatel,
	};

	class Log
	{
	public:
		static void PrintCore(const std::string& message, LogSeverity logSeverity = LogSeverity::LogInfo);
		static void Print(const std::string& message, LogSeverity logSeverity = LogSeverity::LogInfo);
	};

#ifdef SCORCH_DEBUG
#define CORE_ASSERT(x, msg) { if(!(x)) { Log::PrintCore(msg, LogSeverity::LogError); assert(x); } }	
#else
#define CORE_ASSERT(x, ...)
#endif

#ifdef SCORCH_APP_DEBUG
#define APP_ASSERT(x, msg) { if(!(x)) { SC::Log::PrintCore(msg, SC::LogSeverity::LogError); assert(x); } }	
#else
#define APP_ASSERT(x, ...)
#endif
}
