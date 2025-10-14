#pragma once

#include "Core/Base.h"
#include "Core/Log.h"
#include <filesystem>

#ifdef NVIZ_ENABLE_ASSERTS

// Alteratively we could use the same "default" message for both "WITH_MSG" and "NO_MSG" and
// provide support for custom formatting by concatenating the formatting string instead of having the format inside the default message
#define NVIZ_INTERNAL_ASSERT_IMPL(type, check, msg, ...) { if(!(check)) { NVIZ##type##ERROR(msg, __VA_ARGS__); NVIZ_DEBUGBREAK(); } }
#define NVIZ_INTERNAL_ASSERT_WITH_MSG(type, check, ...) NVIZ_INTERNAL_ASSERT_IMPL(type, check, "Assertion failed: {0}", __VA_ARGS__)
#define NVIZ_INTERNAL_ASSERT_NO_MSG(type, check) NVIZ_INTERNAL_ASSERT_IMPL(type, check, "Assertion '{0}' failed at {1}:{2}", NVIZ_STRINGIFY_MACRO(check), std::filesystem::path(__FILE__).filename().string(), __LINE__)

#define NVIZ_INTERNAL_ASSERT_GET_MACRO_NAME(arg1, arg2, macro, ...) macro
#define NVIZ_INTERNAL_ASSERT_GET_MACRO(...) NVIZ_EXPAND_MACRO( NVIZ_INTERNAL_ASSERT_GET_MACRO_NAME(__VA_ARGS__, NVIZ_INTERNAL_ASSERT_WITH_MSG, NVIZ_INTERNAL_ASSERT_NO_MSG) )

// Currently accepts at least the condition and one additional parameter (the message) being optional
#define NVIZ_ASSERT(...) NVIZ_EXPAND_MACRO( NVIZ_INTERNAL_ASSERT_GET_MACRO(__VA_ARGS__)(_, __VA_ARGS__) )
#define NVIZ_CORE_ASSERT(...) NVIZ_EXPAND_MACRO( NVIZ_INTERNAL_ASSERT_GET_MACRO(__VA_ARGS__)(_CORE_, __VA_ARGS__) )
#else
#define NVIZ_ASSERT(...)
#define NVIZ_CORE_ASSERT(...)
#endif
