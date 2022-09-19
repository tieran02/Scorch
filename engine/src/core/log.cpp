#include "core/log.h"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/fmt/ostr.h"

using namespace SC;

std::shared_ptr<spdlog::logger> s_CoreLogger{ nullptr };
std::shared_ptr<spdlog::logger> s_GameLogger{ nullptr };

#define LOG_CORE_TRACE(...) s_CoreLogger->trace(__VA_ARGS__)
#define LOG_CORE_INFO(...) s_CoreLogger->info(__VA_ARGS__)
#define LOG_CORE_WARNING(...) s_CoreLogger->warn(__VA_ARGS__)
#define LOG_CORE_ERROR(...) s_CoreLogger->error(__VA_ARGS__)
#define LOG_CORE_FATAL(...) s_CoreLogger->critical(__VA_ARGS__); abort();

#define LOG_GAME_TRACE(...) s_GameLogger->trace(__VA_ARGS__)
#define LOG_GAME_INFO(...) s_GameLogger->info(__VA_ARGS__)
#define LOG_GAME_WARNING(...) s_GameLogger->warn(__VA_ARGS__)
#define LOG_GAME_ERROR(...) s_GameLogger->error(__VA_ARGS__)
#define LOG_GAME_FATAL(...) s_GameLogger->critical(__VA_ARGS__); abort();

void Log::PrintCore(const std::string& message, LogSeverity severity)
{
	if (!s_CoreLogger)
	{
		s_CoreLogger = spdlog::stdout_color_mt("SCORCH");
#ifdef SCORCH_DEBUG
		s_CoreLogger->set_level(spdlog::level::trace);
#else
		s_CoreLogger->set_level(spdlog::level::info);
#endif
	}

	switch (severity)
	{
	default:
		break;
	case LogSeverity::LogTrace:
		LOG_CORE_TRACE(message);
		break;
	case LogSeverity::LogInfo:
		LOG_CORE_INFO(message);
		break;
	case LogSeverity::LogWarning:
		LOG_CORE_WARNING(message);
		break;
	case LogSeverity::LogError:
		LOG_CORE_ERROR(message);
		break;
	case LogSeverity::LogFatel:
		LOG_CORE_FATAL(message);
		break;
	}
}

void Log::Print(const std::string& message, LogSeverity logSeverity /*= LogSeverity::LogInfo*/)
{
	if (!s_GameLogger)
	{
		s_GameLogger = spdlog::stdout_color_mt("GAME");
		s_GameLogger->set_level(spdlog::level::info);
	}

	switch (logSeverity)
	{
	default:
		break;
	case LogSeverity::LogTrace:
		LOG_GAME_TRACE(message);
		break;
	case LogSeverity::LogInfo:
		LOG_GAME_INFO(message);
		break;
	case LogSeverity::LogWarning:
		LOG_GAME_WARNING(message);
		break;
	case LogSeverity::LogError:
		LOG_GAME_ERROR(message);
		break;
	case LogSeverity::LogFatel:
		LOG_GAME_FATAL(message);
		break;
	}
}
