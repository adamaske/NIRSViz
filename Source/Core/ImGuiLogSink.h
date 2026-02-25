#pragma once

#include "Core/Base.h"

#pragma warning(push, 0)
#include <spdlog/sinks/base_sink.h>
#include <spdlog/details/null_mutex.h>
#pragma warning(pop)

#include <deque>
#include <string>

enum class LogLevel { Trace, Debug, Info, Warn, Error, Critical };

struct ImGuiLogEntry {
	std::string message;
	LogLevel level = LogLevel::Info;
};

/// Lightweight spdlog sink that keeps the last MAX_ENTRIES log messages in memory
/// for display in the ImGui log panel.  Single-threaded (null_mutex) — safe since
/// all logging in NIRSViz happens on the main thread.
class ImGuiLogSink : public spdlog::sinks::base_sink<spdlog::details::null_mutex> {
public:
	static constexpr size_t MAX_ENTRIES = 200;

	static Ref<ImGuiLogSink>& Instance() {
		static Ref<ImGuiLogSink> s_instance = std::make_shared<ImGuiLogSink>();
		return s_instance;
	}

	const std::deque<ImGuiLogEntry>& GetEntries() const { return entries_; }

	void Clear() {
		entries_.clear();
		scroll_to_bottom_ = true;
	}

	bool scroll_to_bottom_ = false;

protected:
	void sink_it_(const spdlog::details::log_msg& msg) override {
		spdlog::memory_buf_t formatted;
		formatter_->format(msg, formatted);
		std::string str = fmt::to_string(formatted);
		if (!str.empty() && str.back() == '\n')
			str.pop_back();

		LogLevel level;
		switch (msg.level) {
			case spdlog::level::trace:    level = LogLevel::Trace;    break;
			case spdlog::level::debug:    level = LogLevel::Debug;    break;
			case spdlog::level::info:     level = LogLevel::Info;     break;
			case spdlog::level::warn:     level = LogLevel::Warn;     break;
			case spdlog::level::err:      level = LogLevel::Error;    break;
			case spdlog::level::critical: level = LogLevel::Critical; break;
			default:                      level = LogLevel::Info;     break;
		}

		if (entries_.size() >= MAX_ENTRIES)
			entries_.pop_front();

		entries_.push_back({ std::move(str), level });
		scroll_to_bottom_ = true;
	}

	void flush_() override {}

private:
	std::deque<ImGuiLogEntry> entries_;
};
