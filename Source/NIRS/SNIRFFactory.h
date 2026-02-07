#pragma once
#include "Core/Base.h"
#include "NIRS/SNIRF.h"
#include "NIRS/SNIRFLoader.h"
#include "NIRS/SNIRFType.h"
#include "NIRS/SNIRFError.h"

#include <filesystem>
#include <vector>

class SNIRFFactory {
public:
    /// Creates a fully-populated SNIRF from a file on disk.
    static bool Create(SNIRF& out_snirf,
                       SNIRFType type,
                       const std::filesystem::path& filepath,
                       std::vector<SNIRFError>& out_errors)
    {
        return NIRS::LoadSNIRF(filepath, out_snirf, out_errors);
    }
};
