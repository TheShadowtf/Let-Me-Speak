#pragma once

#include "GLog.h"

// ✅ Fix macros to support multiple arguments using `__VA_ARGS__`
#define GLOG_INFO(...)  GLog::log(GLogLevel::GLOG_INFO, __VA_ARGS__)
#define GLOG_WARN(...)  GLog::log(GLogLevel::GLOG_WARN, __VA_ARGS__)
#define GLOG_ERROR(...) GLog::log(GLogLevel::GLOG_ERROR, __VA_ARGS__)
#define GLOG_DEBUG(...) GLog::log(GLogLevel::GLOG_DEBUG, __VA_ARGS__)
