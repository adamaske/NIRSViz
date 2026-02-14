#include "pch.h"
#include "Services/SessionService.h"

#include "Services/SNIRFService.h"
#include "Services/AnatomyService.h"

bool SessionService::LoadSubject(const SubjectData& data) {
	// Load SNIRF
	//if (!snirf_.LoadSNIRF()) {
	//	NVIZ_ERROR("Failed to load SNIRF file for subject.");
	//	return false;
	//}
	//// Load Anatomy
	//anatomy_.LoadHead(data.head_path);
	//anatomy_.LoadCortex(data.cortex_path);
	//return true;
	return false;
}

void SessionService::UnloadSubject() {
	// TODO : Implement unloading logic for SNIRF and anatomy data
}

bool SessionService::IsSubjectLoaded() const {
	// TODO : Implement logic to check if a subject is currently loaded
	return false;
}

	