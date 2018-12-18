

#include "spdlog/spdlog.h"
#include "spdlog/sinks/msvc_sink.h"
//
// enable/disable log calls at compile time according to global level.
//
#define SPDLOG_ACTIVE_LEVEL  SPDLOG_LEVEL_WARN,
// SPDLOG_LEVEL_ERROR,
// SPDLOG_LEVEL_CRITICAL,
// SPDLOG_LEVEL_OFF

#define TRACE(...) SPDLOG_TRACE(__VA_ARGS__);
#define DEBUG(...) SPDLOG_DEBUG(__VA_ARGS__);
#define INFO(...) SPDLOG_INFO(__VA_ARGS__);
#define WARN(...) SPDLOG_WARN(__VA_ARGS__);
#define ERROR(...) SPDLOG_ERROR(__VA_ARGS__);
#define CRITICAL(...) CRITICAL(__VA_ARGS__);
