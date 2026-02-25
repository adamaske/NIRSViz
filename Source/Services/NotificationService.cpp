#include "pch.h"
#include "Services/NotificationService.h"

#include <imgui.h>
#include <algorithm>

NotificationService& NotificationService::Get() {
	static NotificationService s_instance;
	return s_instance;
}

void NotificationService::Notify(const std::string& msg, NotificationLevel level, float timeout) {
	notifications_.push_back({ msg, level, timeout, 0.0f });
}

void NotificationService::NotifyWarning(const std::string& msg) {
	Notify(msg, NotificationLevel::Warning, 5.0f);
}

void NotificationService::NotifyError(const std::string& msg) {
	Notify(msg, NotificationLevel::Error, 0.0f);
}

void NotificationService::RenderToasts() {
	if (notifications_.empty()) return;

	const float dt = ImGui::GetIO().DeltaTime;

	// Advance timers
	for (auto& n : notifications_) {
		if (n.timeout > 0.0f)
			n.elapsed += dt;
	}

	// Remove expired auto-dismiss notifications
	notifications_.erase(
		std::remove_if(notifications_.begin(), notifications_.end(),
			[](const Notification& n) { return n.timeout > 0.0f && n.elapsed >= n.timeout; }),
		notifications_.end());

	if (notifications_.empty()) return;

	const ImGuiIO& io        = ImGui::GetIO();
	const float padding      = 10.0f;
	const float toast_width  = 400.0f;
	const float toast_height = 40.0f;
	float pos_y = io.DisplaySize.y - padding;

	int to_remove = -1;

	for (int i = (int)notifications_.size() - 1; i >= 0; i--) {
		const auto& n = notifications_[i];
		pos_y -= toast_height + padding;

		ImGuiWindowFlags flags =
			ImGuiWindowFlags_NoDecoration  |
			ImGuiWindowFlags_NoMove        |
			ImGuiWindowFlags_NoSavedSettings |
			ImGuiWindowFlags_NoBringToFrontOnFocus |
			ImGuiWindowFlags_NoFocusOnAppearing |
			ImGuiWindowFlags_NoNav;

		// Allow input for pinned error toasts so the dismiss button works
		if (n.timeout > 0.0f)
			flags |= ImGuiWindowFlags_NoInputs;

		ImGui::SetNextWindowPos(
			ImVec2(io.DisplaySize.x - toast_width - padding, pos_y),
			ImGuiCond_Always);
		ImGui::SetNextWindowSize(
			ImVec2(toast_width, toast_height),
			ImGuiCond_Always);
		ImGui::SetNextWindowBgAlpha(0.88f);

		const std::string wid = "##Toast_" + std::to_string(i);
		ImGui::Begin(wid.c_str(), nullptr, flags);

		ImVec4 color;
		const char* prefix;
		switch (n.level) {
			case NotificationLevel::Error:
				color  = ImVec4(1.0f, 0.4f, 0.4f, 1.0f);
				prefix = "[ERROR] ";
				break;
			case NotificationLevel::Warning:
				color  = ImVec4(1.0f, 0.85f, 0.2f, 1.0f);
				prefix = "[WARN]  ";
				break;
			default:
				color  = ImVec4(0.7f, 0.9f, 1.0f, 1.0f);
				prefix = "[INFO]  ";
				break;
		}

		ImGui::PushStyleColor(ImGuiCol_Text, color);
		ImGui::Text("%s%s", prefix, n.message.c_str());
		ImGui::PopStyleColor();

		if (n.timeout == 0.0f) {
			ImGui::SameLine(toast_width - 40.0f);
			if (ImGui::SmallButton(" X "))
				to_remove = i;
		}

		ImGui::End();
	}

	if (to_remove >= 0)
		notifications_.erase(notifications_.begin() + to_remove);
}
