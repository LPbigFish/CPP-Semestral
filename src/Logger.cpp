#include "Logger.hpp"

#include <chrono>
#include <ctime>
#include <iostream>
#include <string>
#include <string_view>

namespace {

auto current_timestamp() -> std::string {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    std::tm tm_buf{};
    localtime_r(&time_t, &tm_buf);
    std::string buf(20, '\0');
    std::strftime(buf.data(), buf.size(), "%Y-%m-%d %H:%M:%S", &tm_buf);
    buf.resize(std::strlen(buf.data()));
    return buf;
}

auto level_prefix(LogLevel level) -> std::string_view {
    switch (level) {
        case LogLevel::DEBUG: return "[DEBUG] ";
        case LogLevel::INFO: return "[INFO] ";
        case LogLevel::WARNING: return "[WARNING] ";
        case LogLevel::ERROR: return "[ERROR] ";
    }
    return "";
}

auto level_tag(LogLevel level) -> std::string_view {
    switch (level) {
        case LogLevel::DEBUG: return "DEBUG";
        case LogLevel::INFO: return "INFO";
        case LogLevel::WARNING: return "WARNING";
        case LogLevel::ERROR: return "ERROR";
    }
    return "";
}

} // namespace

auto Logger::init(bool verbose) -> void {
    verbose_ = verbose;

    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    std::tm tm_buf{};
    localtime_r(&time_t, &tm_buf);
    std::string filename(32, '\0');
    auto len = std::strftime(
        filename.data(), filename.size(), "miner_%Y-%m-%d_%H-%M-%S.log", &tm_buf
    );
    filename.resize(len);

    log_file.open(filename);
    if (!log_file.is_open()) {
        std::cerr << "[ERROR] Failed to open log file: " << filename << '\n';
        return;
    }

    log(LogLevel::INFO, "Logger initialized, writing to " + filename);
}

auto Logger::log(LogLevel level, std::string_view message) -> void {
    std::lock_guard lock{log_mutex};

    const auto timestamp = current_timestamp();
    const auto prefix = level_prefix(level);
    const auto tag = level_tag(level);

    if (log_file.is_open()) {
        log_file << '[' << timestamp << "] [" << tag << "] " << message << '\n';
        log_file.flush();
    }

    const auto should_print_console = level != LogLevel::DEBUG || verbose_;
    if (should_print_console) {
        if (level == LogLevel::ERROR) {
            std::cerr << prefix << message << '\n';
        } else {
            std::cout << prefix << message << '\n';
        }
    }
}
