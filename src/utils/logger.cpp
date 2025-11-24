#include "logger.hpp"
#include <iostream>
#include <chrono>
#include <iomanip>
#include <sstream>

Logger& Logger::getInstance() {
    static Logger instance;
    return instance;
}

void Logger::init(const std::string& filename, LogLevel level) {
    std::lock_guard<std::mutex> lock(logMutex);

    if (initialized) {
        return;
    }

    logFile = std::make_unique<std::ofstream>(filename, std::ios::app);
    if (!logFile->is_open()) {
        std::cerr << "Failed to open log file: " << filename << std::endl;
        return;
    }

    currentLevel = level;
    initialized = true;

    *logFile << "[" << getCurrentTimestamp() << "] [INFO] Logger initialized - Log level: "
             << levelToString(level) << std::endl;
    logFile->flush();
}

void Logger::debug(const char* format, ...) {
    if (currentLevel <= LogLevel::DEBUG) {
        va_list args;
        va_start(args, format);
        log(LogLevel::DEBUG, format, args);
        va_end(args);
    }
}

void Logger::info(const char* format, ...) {
    if (currentLevel <= LogLevel::INFO) {
        va_list args;
        va_start(args, format);
        log(LogLevel::INFO, format, args);
        va_end(args);
    }
}

void Logger::warning(const char* format, ...) {
    if (currentLevel <= LogLevel::WARNING) {
        va_list args;
        va_start(args, format);
        log(LogLevel::WARNING, format, args);
        va_end(args);
    }
}

void Logger::error(const char* format, ...) {
    if (currentLevel <= LogLevel::ERROR) {
        va_list args;
        va_start(args, format);
        log(LogLevel::ERROR, format, args);
        va_end(args);
    }
}

void Logger::log(LogLevel level, const char* format, va_list args) {
    std::lock_guard<std::mutex> lock(logMutex);
    if (!initialized) {
        init();
    }

    char buffer[1024];
    vsnprintf(buffer, sizeof(buffer), format, args);

    std::string levelStr = levelToString(level);

    if (logFile && logFile->is_open()) {
        *logFile << "[" << getCurrentTimestamp() << "] [" << levelStr << "] " << buffer << std::endl;
        logFile->flush();
    }

    if (level >= LogLevel::WARNING) {
        std::cout << "[" << getCurrentTimestamp() << "] [" << levelStr << "] " << buffer << std::endl;
    }
}

// construct timestamp in YYYY-MM-DD HH:MM:SS.microseconds(6) format
std::string Logger::getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto us = std::chrono::duration_cast<std::chrono::microseconds>(
        now.time_since_epoch()) % 1000000;

    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
    ss << "." << std::setfill('0') << std::setw(6) << us.count();

    return ss.str();
}

std::string Logger::levelToString(LogLevel level) {
    switch (level) {
        case LogLevel::DEBUG:   return "DEBUG";
        case LogLevel::INFO:    return "INFO";
        case LogLevel::WARNING: return "WARN";
        case LogLevel::ERROR:   return "ERROR";
        default:                return "UNKNOWN";
    }
}

Logger::~Logger() {
    if (logFile && logFile->is_open()) {
        std::string timestamp = getCurrentTimestamp();
        *logFile << "[" << timestamp << "] [INFO] Logger shutting down" << std::endl;
        logFile->flush();
        logFile->close();
    }
}
