#pragma once

#include <cstdint>
#include <format>
#include <fstream>
#include <mutex>
#include <string_view>

enum class LogLevel : uint8_t {
    DEBUG,
    INFO,
    WARNING,
    ERROR,
};

class Logger {
    std::ofstream log_file;
    std::mutex log_mutex;
    bool verbose_{false};

    auto log(LogLevel level, std::string_view message) -> void;

  public:
    auto init(bool verbose) -> void;

    template<class... Args>
    auto debug(std::format_string<Args...> fmt, Args&&... args) -> void {
        log(LogLevel::DEBUG, std::format(fmt, std::forward<Args>(args)...));
    }

    template<class... Args>
    auto info(std::format_string<Args...> fmt, Args&&... args) -> void {
        log(LogLevel::INFO, std::format(fmt, std::forward<Args>(args)...));
    }

    template<class... Args>
    auto warning(std::format_string<Args...> fmt, Args&&... args) -> void {
        log(LogLevel::WARNING, std::format(fmt, std::forward<Args>(args)...));
    }

    template<class... Args>
    auto error(std::format_string<Args...> fmt, Args&&... args) -> void {
        log(LogLevel::ERROR, std::format(fmt, std::forward<Args>(args)...));
    }

    static auto instance() -> Logger& {
        static Logger logger;
        return logger;
    }
};
