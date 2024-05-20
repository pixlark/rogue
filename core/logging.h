#pragma once

#include <cstdio>
#include <string>
#include <format>

#include "services.h"

enum class LogLevel {
    Always,
    Error,
    Warning,
    Info,
    Debug,
    Trace,
};

class ILoggerService {
public:
    virtual void log(LogLevel level, const std::string_view& format, bool newline, std::format_args args) = 0;
};

class Log : public ILoggerService {
    std::FILE* file_handle;

    void write_preamble();

    template <typename ...Args>
    void instance_log_with_level(LogLevel level, const std::string_view& format, bool newline, Args... args) {
        auto packed_args = std::make_format_args(args...);
        this->log(level, format, newline, packed_args);
    }

public:
    Log(LogLevel level, const std::string& path);
    ~Log();

    void log(LogLevel level, const std::string_view& format, bool newline, std::format_args args);

    template<typename ...Args>
    static void log_with_level(LogLevel level, const std::string_view& format, bool newline, Args... args) {
        auto packed_args = std::make_format_args(args...);
        Services::get<ILoggerService>()->log(LogLevel::Error, format, newline, packed_args);
    }

    template<typename ...Args>
    static void always(const std::string_view& format, Args... args) {
        Log::log_with_level(LogLevel::Always, format, true, args...);
    }

    template<typename ...Args>
    static void error(const std::string_view& format, Args... args) {
        Log::log_with_level(LogLevel::Error, format, true, args...);
    }

    template<typename ...Args>
    static void warning(const std::string_view& format, Args... args) {
        Log::log_with_level(LogLevel::Warning, format, true, args...);
    }
    
    template<typename ...Args>
    static void info(const std::string_view& format, Args... args) {
        Log::log_with_level(LogLevel::Info, format, true, args...);
    }

    template<typename ...Args>
    static void debug(const std::string_view& format, Args... args) {
        Log::log_with_level(LogLevel::Debug, format, true, args...);
    }

    template<typename ...Args>
    static void trace(const std::string_view& format, Args... args) {
        Log::log_with_level(LogLevel::Trace, format, true, args...);
    }
};
