#pragma once

#include <stdexcept>
#include <string>
#include <fmt/core.h> // Assuming you are using fmtlib for formatting

// Make sure your NVIZ_ERROR macro is defined here or included from elsewhere
// For example, if NVIZ_ERROR just logs a message:
// #define NVIZ_ERROR(...) /* implementation to log the formatted message */

/**
 * NVIZ_RUNTIME_ERROR: A centralized macro for handling unrecoverable runtime failures.
 * * Args:
 * format_string: A fmt-style format string (e.g., "Resource {} not found.")
 * ...args: The arguments to be formatted into the string.
 *
 * Actions:
 * 1. Formats the message using fmt::format.
 * 2. Logs the error using your existing NVIZ_ERROR infrastructure.
 * 3. Throws a std::runtime_error, terminating the current function's execution.
 */
#define NVIZ_RUNTIME_ERROR(format_string, ...) \
    do { \
        /* 1. Format the message */ \
        std::string error_msg = fmt::format((format_string), __VA_ARGS__); \
        \
        /* 2. Log the error using your existing system */ \
        /* We prefix the error type for clarity in the log */ \
        NVIZ_ERROR("Runtime Error: {}", error_msg); \
        \
        /* 3. Throw the exception */ \
        throw std::runtime_error(error_msg); \
    } while (0)
