#pragma once

#include "Core/Base.h"
#include <vector>
#include <string>

enum class NotificationLevel { Info, Warning, Error };

struct Notification {
	std::string message;
	NotificationLevel level = NotificationLevel::Info;
	float timeout  = 3.0f;  // seconds; 0 = requires manual dismissal
	float elapsed  = 0.0f;
};

/// Toast notification overlay.  Call Notify() from anywhere; RenderToasts() is
/// called once per frame from ImGuiSystem::End() to draw the overlay windows.
class NotificationService {
public:
	static NotificationService& Get();

	void Notify(const std::string& msg,
	            NotificationLevel level = NotificationLevel::Info,
	            float timeout = 3.0f);

	void NotifyWarning(const std::string& msg);
	void NotifyError(const std::string& msg);   // timeout = 0, requires dismissal

	/// Called from ImGuiSystem::End() each frame.
	void RenderToasts();

private:
	std::vector<Notification> notifications_;
};
