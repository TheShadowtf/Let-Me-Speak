#pragma once
#include "GLogConfig.h"
#include <iostream>
#include <fstream>
#include <mutex>
#include <sstream>
#include <chrono>
#include <ctime>
#include <queue>
#include <thread>
#include <condition_variable>
#include <vector>
#include <memory>
#include <filesystem>
#include <fmt/core.h>
#include <atomic>
#include <unordered_map>
#include "GLogUtils.h"

class GLogSink {
public:
    virtual void write(const std::string& message) = 0;
    virtual ~GLogSink() = default;
};

class ConsoleSink : public GLogSink {
    public:
        void write(const std::string& message) override {
            setConsoleColor(detectLogLevel(message));
            std::cout << message;
            resetConsoleColor();
        }
    
    private:
        GLogLevel detectLogLevel(const std::string& message) {
            if (message.find("[ERROR]") != std::string::npos) return GLogLevel::GLOG_ERROR;
            if (message.find("[WARNING]") != std::string::npos) return GLogLevel::GLOG_WARN;
            if (message.find("[INFO]") != std::string::npos) return GLogLevel::GLOG_INFO;
            if (message.find("[DEBUG]") != std::string::npos) return GLogLevel::GLOG_DEBUG;
            return GLogLevel::GLOG_INFO;
        }
    };

    class FileSink : public GLogSink {
        private:
            std::ofstream file;
        
        public:
            FileSink(const std::string& filename) {
                file.open(filename, std::ios::out | std::ios::app);
                if (!file.is_open()) {
                    std::cerr << "[GLog] ERROR: Failed to open log file: " << filename << std::endl;
                }
            }
        
            bool isOpen() const { return file.is_open(); }
        
            void write(const std::string& message) override {
                if (file.is_open()) {
                    file << message;
                    file.flush();
                } else {
                    std::cerr << "[GLog] ERROR: Log file is not open!" << std::endl;
                }
            }
        };

class GLog {
private:
    static std::mutex logMutex;
    static std::queue<std::string> logQueue;
    static std::condition_variable logCondition;
    static std::thread workerThread;
    static bool stopWorker;
    static std::atomic<bool> isInitialized;
    static std::vector<std::shared_ptr<GLogSink>> sinks;
    static std::string logPattern;
    static void workerLoop();
    static void rotateLogFile();
    static bool shouldLog(GLogLevel level);
    static std::string getTimestamp();
    static std::string logLevelToString(GLogLevel level);
    static std::string getThreadID();
    static std::string getLogLevelColor(GLogLevel level);

public:
    static void init(const std::string& filename = "glog.txt");
    static void addSink(std::shared_ptr<GLogSink> sink);
    static void setLogLevel(GLogLevel level);
    static void close();
    static void setPattern(const std::string& pattern);

    template <typename... Args>
    static void log(GLogLevel level, fmt::format_string<Args...> fmtStr, Args&&... args) {
        if (!isInitialized.load()) {
            std::cerr << "[GLog] WARNING: Attempted to log after GLog::close()!" << std::endl;
            return;
        }
        if (!shouldLog(level)) return;

        std::string message = fmt::format(fmtStr, std::forward<Args>(args)...);
        std::string formattedMessage = formatLogEntry(level, message);

        std::lock_guard<std::mutex> lock(logMutex);
        for (auto& sink : sinks) {
            if (sink) {
                sink->write(formattedMessage + "\n");
            }
        }
    }

private:
    static std::string formatLogEntry(GLogLevel level, const std::string& message) {
        std::lock_guard<std::mutex> lock(logMutex);

        std::unordered_map<std::string, std::string> replacements = {
            {"%Y", getTimestamp().substr(1, 4)},
            {"%m", getTimestamp().substr(6, 2)},
            {"%d", getTimestamp().substr(9, 2)},
            {"%H", getTimestamp().substr(12, 2)},
            {"%M", getTimestamp().substr(15, 2)},
            {"%S", getTimestamp().substr(18, 2)},
            {"%e", "000"},  // No millisecond support yet
            {"%l", logLevelToString(level)},
            {"%v", message},
            {"%t", getThreadID()},
            {"%n", "GLog"}  // Logger name (not used in GLog)
        };

        std::string formattedPattern = logPattern;
        
        // Handle color formatting (%^ and %$)
        std::string colorCode = getLogLevelColor(level);
        size_t pos = formattedPattern.find("%^");
        if (pos != std::string::npos) {
            formattedPattern.replace(pos, 2, colorCode);
        }
        pos = formattedPattern.find("%$");
        if (pos != std::string::npos) {
            formattedPattern.replace(pos, 2, "\033[0m");  // Reset color
        }

        // Replace all other placeholders
        for (const auto& [key, value] : replacements) {
            size_t pos = 0;
            while ((pos = formattedPattern.find(key, pos)) != std::string::npos) {
                formattedPattern.replace(pos, key.length(), value);
                pos += value.length();
            }
        }

        return formattedPattern;
    }
};

// ✅ Default pattern: Same as `spdlog`
inline std::string GLog::logPattern = "[%Y-%m-%d %H:%M:%S] [%l] %v";

// ✅ Thread-safe pattern setting
inline void GLog::setPattern(const std::string& pattern) {
    std::lock_guard<std::mutex> lock(logMutex);
    logPattern = pattern;
}

// ✅ Gets thread ID
inline std::string GLog::getThreadID() {
    std::ostringstream ss;
    ss << std::this_thread::get_id();
    return ss.str();
}

// ✅ Gets color codes for log levels
inline std::string GLog::getLogLevelColor(GLogLevel level) {
#ifdef _WIN32
    switch (level) {
        case GLogLevel::GLOG_INFO: return "\033[1;33m";   // Yellow
        case GLogLevel::GLOG_WARN: return "\033[1;35m";   // Magenta
        case GLogLevel::GLOG_ERROR: return "\033[1;31m";  // Red
        case GLogLevel::GLOG_DEBUG: return "\033[1;32m";  // Green
        default: return "";
    }
#else
    switch (level) {
        case GLogLevel::GLOG_INFO: return "\033[1;33m";   // Yellow
        case GLogLevel::GLOG_WARN: return "\033[1;35m";   // Magenta
        case GLogLevel::GLOG_ERROR: return "\033[1;31m";  // Red
        case GLogLevel::GLOG_DEBUG: return "\033[1;32m";  // Green
        default: return "";
    }
#endif
}
