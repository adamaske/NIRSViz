#pragma once
#include "Core/Base.h"
#include "NIRS/Snirf_NEW.h"

#include "NIRS/SNIRFType.h"

class SNIRFFactory {
public:
    static SNIRF CreateSNIRF(SNIRFType type, std::filesystem::path filepath);
};