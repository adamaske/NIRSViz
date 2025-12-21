#pragma once
#include "Core/Base.h"
#include "NIRS/Snirf.h"

#include "NIRS/SNIRFType.h"

class SNIRF;

class SNIRFFactory {
public:
	const SNIRF& CreateSNIRF(SNIRFType type, std::filesystem::path path);
};