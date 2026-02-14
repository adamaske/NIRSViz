#include "SNIRFService.h"

#include "Core/FileDialogService.h"
#include "Core/AssetRegistry.h"
#include "NIRS/SNIRFLoader.h"

bool SNIRFService::Load(const std::filesystem::path& path) {
	std::string filepath = path.string();
	SNIRF snirf = {};
	std::vector<SNIRFError> errors;
	if (!NIRS::LoadSNIRF(filepath, snirf, errors)) {
		for (auto& e : errors)
			NVIZ_ERROR("SNIRF load error: {}", e.message);
		return false;
	}
	snirf_ = CreateRef<SNIRF>(snirf);

	return true;
}
void SNIRFService::Unload() {
	snirf_.reset();
};

bool SNIRFService::IsLoaded() const {
	if (!snirf_)
		return false;
	return true;
}
