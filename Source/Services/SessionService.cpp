#include "pch.h"
#include "Services/SessionService.h"

#include "Services/SNIRFService.h"
#include "Services/AnatomyService.h"
#include "Services/NotificationService.h"

bool SessionService::LoadSubject(const SubjectData& data) {
	bool success = true;

	if (!data.snirf_path.empty()) {
		if (!snirf_.Load(data.snirf_path)) {
			const std::string msg = "Failed to load SNIRF: " + data.snirf_path.filename().string();
			NVIZ_ERROR("SessionService: {}", msg);
			NotificationService::Get().NotifyError(msg);
			success = false;
		} else {
			NVIZ_INFO("SessionService: Loaded SNIRF '{}'", data.snirf_path.filename().string());
		}
	}

	if (!data.head_path.empty()) {
		anatomy_.LoadHead(data.head_path);
		NVIZ_INFO("SessionService: Loaded head mesh '{}'", data.head_path.filename().string());
	}

	if (!data.cortex_path.empty()) {
		anatomy_.LoadCortex(data.cortex_path);
		NVIZ_INFO("SessionService: Loaded cortex mesh '{}'", data.cortex_path.filename().string());
	}

	if (success)
		NotificationService::Get().Notify("Subject loaded successfully.");

	return success;
}

void SessionService::UnloadSubject() {
	snirf_.Unload();
}

bool SessionService::IsSubjectLoaded() const {
	return snirf_.IsLoaded();
}

	