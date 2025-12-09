#pragma once

#include <cstddef>

/// <summary>
/// Interface for systems that need to be notified when the projection time changes.
/// This breaks the circular dependency between PlottingSystem and ProjectionSystem.
/// </summary>
class IProjectionTimeSubscriber {
public:
    virtual ~IProjectionTimeSubscriber() = default;

    /// <summary>
    /// Called when the projection time index changes.
    /// </summary>
    /// <param name="index">The new time index</param>
    /// <param name="actualTime">The actual time value</param>
    virtual void OnProjectionTimeChanged(size_t index, double actualTime) = 0;
};
