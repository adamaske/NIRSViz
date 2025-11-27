#include "pch.h"
#include "App/Controllers/TimeController.h"
#include <algorithm>
#include <cmath>

#include "Events/EventBus.h"

TimeController::TimeController()
{
}

void TimeController::Initialize(const std::vector<double>& timeData, double samplingRate)
{
    m_TimeData = timeData;
    m_SamplingRate = samplingRate;

    if (!m_TimeData.empty()) {
        m_StartTime = m_TimeData.front();
        m_EndTime = m_TimeData.back();
        m_CurrentTime = m_StartTime;
        m_CurrentTimeIndex = 0;
        m_Initialized = true;
    }
    else {
        m_Initialized = false;
    }

    m_AccumulatedTime = 0.0;
    m_PlaybackState = PlaybackState::STOPPED;
}

void TimeController::Reset()
{
    m_CurrentTime = m_StartTime;
    m_CurrentTimeIndex = 0;
    m_AccumulatedTime = 0.0;
    m_PlaybackState = PlaybackState::STOPPED;
    NotifyTimeChanged();
}

void TimeController::SetCurrentTime(double time)
{
    if (!m_Initialized) return;

    // Clamp to valid range
    time = std::clamp(time, m_StartTime, m_EndTime);

    size_t newIndex = TimeToIndex(time);

    if (newIndex != m_CurrentTimeIndex) {
        m_CurrentTimeIndex = newIndex;
        m_CurrentTime = IndexToTime(newIndex);
        NotifyTimeChanged();
    }
}

void TimeController::SetCurrentTimeIndex(size_t index)
{
    if (!m_Initialized) return;

    if (IsValidTimeIndex(index) && index != m_CurrentTimeIndex) {
        m_CurrentTimeIndex = index;
        m_CurrentTime = IndexToTime(index);
        NotifyTimeChanged();
    }
}

void TimeController::StepForward()
{
    if (!m_Initialized) return;

    if (m_CurrentTimeIndex < m_TimeData.size() - 1) {
        SetCurrentTimeIndex(m_CurrentTimeIndex + 1);
    }
}

void TimeController::StepBackward()
{
    if (!m_Initialized) return;

    if (m_CurrentTimeIndex > 0) {
        SetCurrentTimeIndex(m_CurrentTimeIndex - 1);
    }
}

void TimeController::SeekToStart()
{
    SetCurrentTimeIndex(0);
}

void TimeController::SeekToEnd()
{
    if (!m_Initialized) return;
    SetCurrentTimeIndex(m_TimeData.size() - 1);
}

size_t TimeController::TimeToIndex(double time) const
{
    if (!m_Initialized || m_TimeData.empty()) return 0;

    // Use sampling rate for direct conversion
    size_t index = static_cast<size_t>(std::round((time - m_StartTime) * m_SamplingRate));

    // Clamp to valid range
    return std::clamp(index, size_t(0), m_TimeData.size() - 1);
}

double TimeController::IndexToTime(size_t index) const
{
    if (!m_Initialized || index >= m_TimeData.size()) return 0.0;

    return m_TimeData[index];
}

bool TimeController::IsValidTimeIndex(size_t index) const
{
    return m_Initialized && index < m_TimeData.size();
}

bool TimeController::IsValidTime(double time) const
{
    return m_Initialized && time >= m_StartTime && time <= m_EndTime;
}

void TimeController::Play()
{
    if (!m_Initialized) return;

    // If stopped, reset to beginning (or end if playing backward)
    if (m_PlaybackState == PlaybackState::STOPPED) {
        if (m_PlaybackDirection > 0) {
            SeekToStart();
        }
        else {
            SeekToEnd();
        }
    }

    m_PlaybackState = PlaybackState::PLAYING;
}

void TimeController::Pause()
{
    if (m_PlaybackState == PlaybackState::PLAYING) {
        m_PlaybackState = PlaybackState::PAUSED;
    }
}

void TimeController::Stop()
{
    m_PlaybackState = PlaybackState::STOPPED;
    m_AccumulatedTime = 0.0;
    SeekToStart();
}

void TimeController::TogglePlayPause()
{
    if (IsPlaying()) {
        Pause();
    }
    else {
        Play();
    }
}

void TimeController::Update(float deltaTime)
{
    if (!m_Initialized || m_PlaybackState != PlaybackState::PLAYING) {
        return;
    }

    // Accumulate time scaled by playback speed
    m_AccumulatedTime += deltaTime * m_PlaybackSpeed;

    // Calculate how much time has passed in the actual data timeline
    double timeStep = 1.0 / m_SamplingRate;

    // Advance time index based on accumulated time
    while (m_AccumulatedTime >= timeStep) {
        m_AccumulatedTime -= timeStep;

        // Move to next frame based on direction
        if (m_PlaybackDirection > 0) {
            if (m_CurrentTimeIndex < m_TimeData.size() - 1) {
                SetCurrentTimeIndex(m_CurrentTimeIndex + 1);
            }
            else {
                // Reached end
                HandleLoopBehavior();
                break;
            }
        }
        else {
            if (m_CurrentTimeIndex > 0) {
                SetCurrentTimeIndex(m_CurrentTimeIndex - 1);
            }
            else {
                // Reached start
                HandleLoopBehavior();
                break;
            }
        }
    }
}

void TimeController::HandleLoopBehavior()
{
    switch (m_LoopMode) {
    case PlaybackLoopMode::NONE:
        Stop();
        break;

    case PlaybackLoopMode::LOOP:
        if (m_PlaybackDirection > 0) {
            SeekToStart();
        }
        else {
            SeekToEnd();
        }
        break;

    case PlaybackLoopMode::PING_PONG:
        // Reverse direction
        m_PlaybackDirection *= -1;
        break;
    }
}

void TimeController::NotifyTimeChanged()
{
	OnTimeChanged event;
	event.NewTime = m_CurrentTime;
	event.NewTimeIndex = m_CurrentTimeIndex;
    EventBus::Instance().Publish<OnTimeChanged>(event);

    if (m_OnTimeChanged) {
        m_OnTimeChanged(m_CurrentTimeIndex, m_CurrentTime);
    }
}