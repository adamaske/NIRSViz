#pragma once
#include "Core/Base.h"
#include "NIRS/SNIRF.h"
#include "NIRS/SNIRFProvider.h"

class SNIRFService : public NIRS::ISNIRFProvider {
public:
	// Load a SNIRF file from disk
	bool Load(const std::filesystem::path& path);

	// Unload the current SNIRF
	void Unload();

	// Access ( never null after successful Load )
	const Ref<SNIRF>& GetSNIRF() const {
		return snirf_;
	};

	bool IsLoaded() const;

	const Ref<SNIRF>& GetLoadedSNIRF() override {
		return snirf_;
	};

private:
	Ref<SNIRF> snirf_;
};