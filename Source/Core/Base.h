#pragma once
#include <memory>

#define NVIZ_ENABLE_ASSERTS

#if defined(_MSC_VER)
#define NVIZ_DEBUGBREAK() __debugbreak()
#elif defined(__clang__) || defined(__GNUC__)
#define NVIZ_DEBUGBREAK() __builtin_trap()
#else
// Fallback for other compilers
#include <cstdlib>
#define NVIZ_DEBUGBREAK() std::abort()
#endif

#define NVIZ_EXPAND_MACRO(x) x
#define NVIZ_STRINGIFY_MACRO(x) #x

#define BIT(x) (1 << x)

#define BIND_EVENT_FN(fn) [this](auto&&... args) -> decltype(auto) { return this->fn(std::forward<decltype(args)>(args)...); }

template<typename T>
using Scope = std::unique_ptr<T>;
template<typename T, typename ... Args>
constexpr Scope<T> CreateScope(Args&& ... args)
{
	return std::make_unique<T>(std::forward<Args>(args)...);
}

template<typename T>
using Ref = std::shared_ptr<T>;
template<typename T, typename ... Args>
constexpr Ref<T> CreateRef(Args&& ... args)
{
	return std::make_shared<T>(std::forward<Args>(args)...);
}

using DeltaTime = double;

#define MAX_HITS 256

#include "Core/Except.h"
#include "Core/Log.h"
#include "Core/Assert.h"
#include "Core/Profiler.h"