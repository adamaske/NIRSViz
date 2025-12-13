#pragma once

#include <chrono>
#include <string>
#include "Core/Log.h"

// Simple RAII-based profiler that logs function execution time
class ScopedTimer {
public:
    ScopedTimer(const char* name)
        : name_(name), start_(std::chrono::high_resolution_clock::now()) {}

    ~ScopedTimer() {
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start_);
        double ms = duration.count() / 1000.0;
        NVIZ_DEBUG("[PROFILE] {}: {:.3f} ms", name_, ms);
    }

private:
    const char* name_;
    std::chrono::time_point<std::chrono::high_resolution_clock> start_;
};

#define NVIZ_PROFILE_FUNCTION() ScopedTimer timer__(__FUNCTION__)
#define NVIZ_PROFILE_SCOPE(name) ScopedTimer timer__(name)
