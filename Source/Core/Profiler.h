#pragma once

#include <chrono>
#include <string>
#include <fmt/core.h>
#include "Core/Log.h"

// Simple RAII-based profiler that logs function execution time
class ScopedTimer {
public:
    ScopedTimer(const std::string& name)
        : name_(name), start_(std::chrono::high_resolution_clock::now()) {}
    ScopedTimer(const std::string& name, const std::string& msg)
        : name_(name), msg_(msg), has_msg(true), start_(std::chrono::high_resolution_clock::now()) {}

    ~ScopedTimer() {
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start_);
        double ms = duration.count() / 1000.0;

        if (!has_msg) NVIZ_DEBUG("[PROFILE] {}: {:.3f} ms", name_, ms);
        else NVIZ_DEBUG("[PROFILE] {} | {}: {:.3f} ms", name_, msg_, ms);
    }

private:
    std::string name_;
    std::string msg_;
    std::chrono::time_point<std::chrono::high_resolution_clock> start_;

    bool has_msg = false;
};

#define NVIZ_PROFILE_FUNCTION() ScopedTimer timer__(__FUNCTION__)
#define NVIZ_PROFILE_SCOPE(name) ScopedTimer timer__(name)
#define NVIZ_PROFILE_FUNCTION_MSG(format_string, ...) \
    ScopedTimer timer__(__FUNCTION__, fmt::format((format_string), __VA_ARGS__))
