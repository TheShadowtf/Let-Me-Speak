#include "GLog.h"
#include "GLogUtils.h"
#include <fmt/core.h>

/*
    %Y → Year
    %m → Month
    %d → Day
    %H → Hour
    %M → Minute
    %S → Second
    %e → Millisecond
    %l → Log level
    %v → Log message
    %t → Thread ID
    %n → Logger name
    %^ → Starts color formatting
    %$ → Resets color formatting
*/

std::mutex GLog::logMutex;
std::queue<std::string> GLog::logQueue;
std::condition_variable GLog::logCondition;
std::thread GLog::workerThread;
bool GLog::stopWorker = false;
std::atomic<bool> GLog::isInitialized = false;

std::vector<std::shared_ptr<GLogSink>> GLog::sinks;

void GLog::init(const std::string& filename) {

    if (isInitialized.load()) {
        std::cerr << "[GLog] WARNING: GLog::init() already called!\n";
        return;
    }

    std::cout << "[GLog] Creating file and console sinks..." << std::endl;
    auto fileSink = std::make_shared<FileSink>(filename);
    auto consoleSink = std::make_shared<ConsoleSink>();

    if (!fileSink || !fileSink->isOpen()) {
        std::cerr << "[GLog] ERROR: Failed to create log file sink!" << std::endl;
        return;
    }

    std::cout << "[GLog] Adding sinks..." << std::endl;
    
    try {
        addSink(fileSink);
        addSink(consoleSink);
    } catch (const std::exception& e) {
        std::cerr << "[GLog] ERROR: Exception in addSink(): " << e.what() << std::endl;
    } catch (...) {
        std::cerr << "[GLog] ERROR: Unknown error in addSink()!" << std::endl;
    }

    std::cout << "[GLog] Sinks added successfully. Total sinks: " << sinks.size() << std::endl;

    if (ENABLE_ASYNC) {
        stopWorker = false;
        workerThread = std::thread(workerLoop);
        std::cout << "[GLog] Async logging thread started!" << std::endl;
    }

    isInitialized.store(true);
    std::cout << "[GLog] Initialization complete." << std::endl;
}


void GLog::addSink(std::shared_ptr<GLogSink> sink) {
    if (!sink) {
        std::cerr << "[GLog] ERROR: Trying to add a NULL sink!" << std::endl;
        return;
    }

    std::cout << "[GLog] Checking if already locked..." << std::endl;

    if (logMutex.try_lock()) {
        logMutex.unlock();  // ✅ Mutex was NOT locked, safe to proceed
        std::cout << "[GLog] Mutex is free, proceeding." << std::endl;
    } else {
        std::cerr << "[GLog] ERROR: Deadlock detected! logMutex is already locked!" << std::endl;
        return;
    }

    std::lock_guard<std::mutex> lock(logMutex);
    sinks.push_back(sink);
    std::cout << "[GLog] Sink added successfully." << std::endl;
}



void GLog::setLogLevel(GLogLevel level) {
    CURRENT_LOG_LEVEL = level;
}

bool GLog::shouldLog(GLogLevel level) {
    return static_cast<int>(level) >= static_cast<int>(CURRENT_LOG_LEVEL);
}

std::string GLog::getTimestamp() {
    auto now = std::chrono::system_clock::now();
    std::time_t nowTime = std::chrono::system_clock::to_time_t(now);
    std::tm localTime;

    #ifdef _WIN32
        localtime_s(&localTime, &nowTime);
    #else
        localtime_r(&nowTime, &localTime);
    #endif

    std::ostringstream oss;
    oss << std::put_time(&localTime, "[%Y-%m-%d %H:%M:%S]");
    return oss.str();
}

std::string GLog::logLevelToString(GLogLevel level) {
    switch (level) {
        case GLogLevel::GLOG_INFO: return "INFO";
        case GLogLevel::GLOG_WARN: return "WARNING";
        case GLogLevel::GLOG_ERROR: return "ERROR";
        case GLogLevel::GLOG_DEBUG: return "DEBUG";
        default: return "UNKNOWN";
    }
}

void GLog::workerLoop() {
    std::cout << "[GLog] Worker thread started.\n";

    while (true) {
        std::unique_lock<std::mutex> lock(logMutex);
        logCondition.wait(lock, [] { return !logQueue.empty() || stopWorker; });

        if (stopWorker) {
            std::cout << "[GLog] Worker thread stopping.\n";
            break;  // ✅ Stop the loop immediately
        }

        while (!logQueue.empty()) {
            std::string logEntry = logQueue.front();
            logQueue.pop();

            for (auto& sink : sinks) {
                if (sink) {
                    sink->write(logEntry);
                } else {
                    std::cerr << "[GLog] ERROR: Null sink in workerLoop!\n";
                }
            }
        }
    }

    std::cout << "[GLog] Worker thread exited cleanly.\n";
}

void GLog::rotateLogFile() {
    std::filesystem::path logPath("glog.txt");

    if (!std::filesystem::exists(logPath)) return;

    size_t logSize = std::filesystem::file_size(logPath) / (1024 * 1024);
    if (logSize < MAX_LOG_SIZE_MB) return;

    // Delete oldest file if limit exceeded
    std::filesystem::path oldestFile = fmt::format("glog_{}.txt", MAX_LOG_FILES);
    if (std::filesystem::exists(oldestFile)) {
        std::filesystem::remove(oldestFile);
    }

    // Shift logs
    for (int i = MAX_LOG_FILES - 1; i >= 1; --i) {
        std::filesystem::path oldFile = fmt::format("glog_{}.txt", i);
        std::filesystem::path newFile = fmt::format("glog_{}.txt", i + 1);
        if (std::filesystem::exists(oldFile)) {
            std::filesystem::rename(oldFile, newFile);
        }
    }

    // Rename current log
    std::filesystem::rename("glog.txt", "glog_1.txt");
}


void GLog::close() {
    std::cout << "[GLog] Closing logging system...\n";

    if (ENABLE_ASYNC) {
        {
            std::lock_guard<std::mutex> lock(logMutex);
            stopWorker = true;
        }

        logCondition.notify_one();

        if (workerThread.joinable()) {
            std::cout << "[GLog] Waiting for worker thread to stop...\n";
            workerThread.join();
            std::cout << "[GLog] Worker thread successfully stopped.\n";
        } else {
            std::cerr << "[GLog] ERROR: Worker thread was not joinable!\n";
        }
    }

    {
        std::lock_guard<std::mutex> lock(logMutex);
        sinks.clear();  // ✅ Ensure no other thread is accessing sinks
    }

    isInitialized.store(false);  // ✅ Mark logging as uninitialized

    std::cout << "[GLog] Logging system shutdown complete.\n";
}
