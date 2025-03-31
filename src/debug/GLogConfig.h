#pragma once  // ✅ Prevents multiple includes

#ifndef GLOG_CONFIG_H
#define GLOG_CONFIG_H

// ✅ Ensure GLogLevel is defined before use
enum class GLogLevel {
    GLOG_INFO,
    GLOG_WARN,
    GLOG_ERROR,
    GLOG_DEBUG
};

// ✅ Default Log Settings (Kept from before)
inline GLogLevel CURRENT_LOG_LEVEL = GLogLevel::GLOG_INFO;
static bool ENABLE_ASYNC = true;
static bool ENABLE_LOG_ROTATION = true;
static size_t MAX_LOG_SIZE_MB = 10; // ✅ Rotate at 10MB
static int MAX_LOG_FILES = 5; // ✅ Keep last 5 logs

#endif // GLOG_CONFIG_H
