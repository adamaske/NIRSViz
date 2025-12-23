#pragma once
#include "Core/Base.h"
#include "NIRS/Snirf_NEW.h"
#include "NIRS/Snirf.h"

#include "NIRS/SNIRFType.h"
#include "SNIRFError.h"

class SNIRFFactory {
public:
	static bool CreateSNIRF(SNIRF& out_snirf, SNIRFType type, std::filesystem::path filepath, std::vector<SNIRFError>& out_errors);

	static SNIRF_NEW CreateSNIRF_NEW(SNIRFType type, std::filesystem::path filepath);
};