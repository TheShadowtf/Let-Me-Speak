#include "GLogUtils.h"
#include "GLogConfig.h"
#include <sstream>
#include <ctime>
#include <chrono>
#include <iostream>

#ifdef _WIN32
    #include <windows.h>  
#endif

std::string getTimestamp() {
    auto now = std::chrono::system_clock::now();
    std::time_t nowTime = std::chrono::system_clock::to_time_t(now);
    std::tm localTime;

#ifdef _WIN32
    localtime_s(&localTime, &nowTime);
#else
    localtime_r(&nowTime, &localTime);
#endif

    char buffer[80];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &localTime);
    return std::string(buffer);
}

std::string logLevelToString(GLogLevel level) {
    switch (level) {
        case GLogLevel::GLOG_INFO: return "[INFO] ";
        case GLogLevel::GLOG_WARN: return "[WARNING] ";
        case GLogLevel::GLOG_ERROR: return "[ERROR] ";
        case GLogLevel::GLOG_DEBUG: return "[DEBUG] ";
        default: return "[UNKNOWN] ";
    }
}

bool shouldLog(GLogLevel level) {
    return static_cast<int>(level) >= static_cast<int>(CURRENT_LOG_LEVEL);
}

void setConsoleColor(GLogLevel level) {
#ifdef _WIN32
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    switch (level) {
        case GLogLevel::GLOG_INFO: SetConsoleTextAttribute(hConsole, 14); break;  // Yellow
        case GLogLevel::GLOG_WARN: SetConsoleTextAttribute(hConsole, 6); break;   // Orange
        case GLogLevel::GLOG_ERROR: SetConsoleTextAttribute(hConsole, 12); break; // Red
        case GLogLevel::GLOG_DEBUG: SetConsoleTextAttribute(hConsole, 10); break; // Green
    }
#else
    switch (level) {
        case GLogLevel::GLOG_INFO: std::cout << "\033[1;33m"; break;  // Yellow
        case GLogLevel::GLOG_WARN: std::cout << "\033[1;31m"; break;  // Red
        case GLogLevel::GLOG_ERROR: std::cout << "\033[1;41m"; break; // Red background
        case GLogLevel::GLOG_DEBUG: std::cout << "\033[1;32m"; break; // Green
    }
#endif
}

void resetConsoleColor() {
#ifdef _WIN32
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);
#else
    std::cout << "\033[0m";
#endif
}
    