#pragma once

#include <functional>

enum class PlaybackState {
    STOPPED = 0,
    PLAYING = 1,
    PAUSED = 2
};

enum class PlaybackLoopMode {
    NONE = 0,      // Stop at end
    LOOP = 1,      // Restart from beginning
    PING_PONG = 2  // Reverse direction at ends
};

class TimeController {
public:
    TimeController();

    // This class manages the time-related functionality for projection, plotting and playback.

    // ========== Initialization ==========
    // Call this when a new SNIRF file is loaded
    void Initialize(const std::vector<double>& timeData, double samplingRate);
    void Reset();

    // ========== Time Navigation ==========
    void SetCurrentTime(double time);
    void SetCurrentTimeIndex(size_t index);

    double GetCurrentTime() const { return m_CurrentTime; }
    size_t GetCurrentTimeIndex() const { return m_CurrentTimeIndex; }

    // Bounds
    double GetStartTime() const { return m_StartTime; }
    double GetEndTime() const { return m_EndTime; }
    double GetDuration() const { return m_EndTime - m_StartTime; }

    // Step navigation
    void StepForward();
    void StepBackward();
    void SeekToStart();
    void SeekToEnd();

    // ========== Conversion Utilities ==========
    size_t TimeToIndex(double time) const;
    double IndexToTime(size_t index) const;

    bool IsValidTimeIndex(size_t index) const;
    bool IsValidTime(double time) const;

    // ========== Playback Control ==========
    void Play();
    void Pause();
    void Stop();
    void TogglePlayPause();

    bool IsPlaying() const { return m_PlaybackState == PlaybackState::PLAYING; }
    bool IsPaused() const { return m_PlaybackState == PlaybackState::PAUSED; }
    bool IsStopped() const { return m_PlaybackState == PlaybackState::STOPPED; }

    PlaybackState GetPlaybackState() const { return m_PlaybackState; }

    // ========== Playback Settings ==========
    void SetPlaybackSpeed(double speed) { m_PlaybackSpeed = speed; }
    double GetPlaybackSpeed() const { return m_PlaybackSpeed; }

    void SetLoopMode(PlaybackLoopMode mode) { m_LoopMode = mode; }
    PlaybackLoopMode GetLoopMode() const { return m_LoopMode; }

    void SetPlaybackDirection(int direction) { m_PlaybackDirection = direction; }
    int GetPlaybackDirection() const { return m_PlaybackDirection; }

    // ========== Update ==========
    // Call this every frame with delta time to advance playback
    void Update(float deltaTime);

    // ========== Callbacks ==========
    // Register a callback to be notified when time index changes
    using TimeChangedCallback = std::function<void(size_t newIndex, double newTime)>;
    void SetTimeChangedCallback(TimeChangedCallback callback) { m_OnTimeChanged = callback; }

    // ========== Data Access ==========
    const std::vector<double>& GetTimeData() const { return m_TimeData; }
    double GetSamplingRate() const { return m_SamplingRate; }
    size_t GetTimeDataSize() const { return m_TimeData.size(); }

    bool IsInitialized() const { return m_Initialized; }

private:
    void NotifyTimeChanged();
    void HandleLoopBehavior();

    // Time data
    std::vector<double> m_TimeData;
    double m_SamplingRate = 0.0;
    bool m_Initialized = false;

    // Current state
    double m_CurrentTime = 0.0;
    size_t m_CurrentTimeIndex = 0;

    // Bounds
    double m_StartTime = 0.0;
    double m_EndTime = 0.0;

    // Playback state
    PlaybackState m_PlaybackState = PlaybackState::STOPPED;
    double m_PlaybackSpeed = 1.0;  // 1.0 = normal speed, 2.0 = 2x speed, etc.
    int m_PlaybackDirection = 1;   // 1 = forward, -1 = backward
    PlaybackLoopMode m_LoopMode = PlaybackLoopMode::LOOP;

    // Accumulated time for playback
    double m_AccumulatedTime = 0.0;

    // Callback
    TimeChangedCallback m_OnTimeChanged;
};