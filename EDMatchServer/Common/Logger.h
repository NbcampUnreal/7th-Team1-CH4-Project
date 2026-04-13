#pragma once

#include <cstdio>
#include <cstdarg>
#include <ctime>
#include <mutex>

class Logger
{
public:
    enum class Level { Debug, Info, Warn, Error };

    static void Log(Level level, const char* fmt, ...)
    {
        static std::mutex logMutex;
        std::lock_guard<std::mutex> lock(logMutex);

        time_t now = time(nullptr);
        struct tm t;
        localtime_s(&t, &now);

        const char* tag = "";
        switch (level)
        {
        case Level::Debug: tag = "DEBUG"; break;
        case Level::Info:  tag = "INFO";  break;
        case Level::Warn:  tag = "WARN";  break;
        case Level::Error: tag = "ERROR"; break;
        }

        printf("[%02d:%02d:%02d][%s] ", t.tm_hour, t.tm_min, t.tm_sec, tag);

        va_list args;
        va_start(args, fmt);
        vprintf(fmt, args);
        va_end(args);

        printf("\n");
    }
};

#define LOG_DEBUG(fmt, ...) Logger::Log(Logger::Level::Debug, fmt, ##__VA_ARGS__)
#define LOG_INFO(fmt, ...)  Logger::Log(Logger::Level::Info,  fmt, ##__VA_ARGS__)
#define LOG_WARN(fmt, ...)  Logger::Log(Logger::Level::Warn,  fmt, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...) Logger::Log(Logger::Level::Error, fmt, ##__VA_ARGS__)
