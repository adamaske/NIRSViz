#pragma once

#include "NIRS/SNIRF.h"
#include "NIRS/SNIRFError.h"

#include <filesystem>
#include <vector>

namespace NIRS {

    /// Loads a .snirf (HDF5) file and populates every field of `out`.
    /// Returns true on success.  On failure, pushes errors into `out_errors`.
    bool LoadSNIRF(const std::filesystem::path& filepath,
                   SNIRF& out,
                   std::vector<SNIRFError>& out_errors);
}
