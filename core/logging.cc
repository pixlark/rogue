#include "logging.h"

#include <print>
#include <format>
#include <chrono>

//
// Logger
//

Log::Log(LogLevel level, const std::string& path) {
    this->file_handle = std::fopen(path.c_str(), "a");
    this->write_preamble();
}

Log::~Log() {
    std::fclose(this->file_handle);
}

void Log::write_preamble() {
    const std::string preamble_format = R"(
============================================
Game launch at {0:%T} on {0:%F}
============================================
)";
    this->instance_log_with_level(LogLevel::Always, preamble_format, true, std::chrono::system_clock::now());
}

void Log::log(LogLevel level, const std::string_view& format, bool newline, std::format_args args) {
    // TODO(brooke.tilley): test log level

    std::vprint_unicode(format, args);
    std::vprint_unicode(this->file_handle, format, args);
    if (newline) {
        std::println("");
        std::println(this->file_handle, "");
    }
}
