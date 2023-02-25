#pragma once
#include <cstdio>
#include <string>
#include <cassert>

#include "spdlog/fmt/fmt.h"

template<typename ...Args>
inline std::string string_format(fmt::format_string<Args...> fmt, Args && ...args)
{
	return fmt::format(fmt, std::forward<Args>(args)...);
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
