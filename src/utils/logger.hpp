#pragma once

#include <string>
#include <fstream>
#include <memory>
#include <mutex>
#include <cstdarg>

enum class LogLevel {
    DEBUG = 0,
    INFO = 1,
    WARNING = 2,
    ERROR = 3
};

class Logger {
public:
    static Logger& getInstance();
    void init(const std::string& filename = "routing_debug.log", LogLevel level = LogLevel::DEBUG);

    void debug(const char* format, ...);
    void info(const char* format, ...);
    void warning(const char* format, ...);
    void error(const char* format, ...);

    ~Logger();

private:
    Logger() = default;
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    void log(LogLevel level, const char* format, va_list args);
    std::string getCurrentTimestamp();
    std::string levelToString(LogLevel level);

    std::unique_ptr<std::ofstream> logFile;
    LogLevel currentLevel = LogLevel::DEBUG;
    std::mutex logMutex;
    bool initialized = false;
};

#define log_debug(format, ...) Logger::getInstance().debug(format, ##__VA_ARGS__)
#define log_info(format, ...) Logger::getInstance().info(format, ##__VA_ARGS__)
#define log_warning(format, ...) Logger::getInstance().warning(format, ##__VA_ARGS__)
#define log_error(format, ...) Logger::getInstance().error(format, ##__VA_ARGS__)
