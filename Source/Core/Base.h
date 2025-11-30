#pragma once
#include <memory>

#define NVIZ_ENABLE_ASSERTS
#define NVIZ_DEBUGBREAK() __debugbreak()

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

namespace Viewports {

    // 1. Define the underlying type (uint32_t) for efficiency
    enum class ID : uint32_t
    {
        // 2. Define the constants within the enum class scope
        AnatomyViewport = 1,
        MainViewport = 2,
        AltasViewport = 3,
        ProbeEditor = 4,
        ChannelSelector = 5
    };

    using ViewportID = ID;

    constexpr uint32_t GetID(ID viewport)
    {
        return static_cast<uint32_t>(viewport);
    }

}


#define MAX_HITS 256

#include "Core/Log.h"
#include "Core/Assert.h"