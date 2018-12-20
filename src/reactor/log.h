
#pragma once

#include <memory>
#include <iostream>
#include <string>
#include <cstdio>

#include "types.h"

enum class rLogLevel {
	trace,
	debug,
	info,
	warn,
	error,
	fatal,
};

#define RLOG_LEVEL_TRACE rLogLevel::trace
#define RLOG_LEVEL_DEBUG rLogLevel::debug
#define RLOG_LEVEL_INFO rLogLevel::info
#define RLOG_LEVEL_WARN rLogLevel::warn
#define RLOG_LEVEL_ERROR rLogLevel::error
#define RLOG_LEVEL_FATAL rLogLevel::fatal


#define RLOG_ACTIVE_LEVEL RLOG_LEVEL_WARN

void rLog(rLogLevel log_level, const string& format, ... );

#define LOG(level, ...) rLog(level, __VA_ARGS__)
#define TRACE(...) LOG(rLogLevel::trace, __VA_ARGS__)
#define DEBUG(...) LOG(rLogLevel::trace, __VA_ARGS__)
#define INFO(...) LOG(rLogLevel::info, __VA_ARGS__)
#define WARN(...) LOG(rLogLevel::warn, __VA_ARGS__)
#define ERROR(...) LOG(rLogLevel::error, __VA_ARGS__)
#define FATAL(...) LOG(rLogLevel::fatal, __VA_ARGS__)
