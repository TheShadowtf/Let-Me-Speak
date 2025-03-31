#pragma once
#include "GLogConfig.h"  // ✅ Fix: Ensures GLogLevel is defined

#include <string>

std::string getTimestamp();
std::string logLevelToString(GLogLevel level);
bool shouldLog(GLogLevel level);
void setConsoleColor(GLogLevel level);
void resetConsoleColor();
