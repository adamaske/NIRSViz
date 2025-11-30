#pragma once
#include "Core/Base.h"
#include "NIRS/Snirf.h"

enum SNIRFType {
	SNIRF_TYPE_NONE,
	SNIRF_TYPE_UNKNOWN,
	SNIRF_TYPE_HOMER3,
	SNIRF_TYPE_NIRSPY,
	SNIRF_TYPE_SATORI,
	SNIRF_TYPE_AURORA,
	SNIRF_TYPE_MNE_NIRS,
	SNIRF_TYPE_CUSTOM
};

class SNIRF;

class SNIRFFactory {
public:
	const SNIRF& CreateSNIRF(SNIRFType type, std::filesystem::path path);
};