#include "pch.h"

#include "NIRS/NIRS.h"

namespace NIRS {

    std::string LandmarkToString(Landmark landmark) {
        switch (landmark) {
            // --- ANATOMICAL LANDMARKS ---
        case Nz:    return "Nz";
        case Iz:    return "Iz";
        case LPA:   return "LPA";
        case RPA:   return "RPA";

            // --- MIDLINE (Z) ELECTRODES ---
        case Fpz:   return "Fpz";
        case AFz:   return "AFz";
        case Fz:    return "Fz";
        case FCz:   return "FCz";
        case Cz:    return "Cz";
        case CPz:   return "CPz";
        case Pz:    return "Pz";
        case POz:   return "POz";
        case Oz:    return "Oz";

            // --- FRONTAL POLAR (Fp) ---
        case Fp1:   return "Fp1";
        case Fp2:   return "Fp2";
            // ... (Include all Fp cases) ...

            // --- ANTERO-FRONTAL (AF) ---
        case AF1:   return "AF1";
        case AF2:   return "AF2";
        case AF7:   return "AF7";
        case AF8:   return "AF8";
            // ... (Include all AF cases) ...

            // --- CENTRAL (C) ---
        case C1:    return "C1";
        case C2:    return "C2";
        case C3:    return "C3";
        case C4:    return "C4";
        case C5:    return "C5";
        case C6:    return "C6";

            // --- TEMPORAL (T, FT, TP) ---
        case T7:    return "T7";
        case T8:    return "T8";
        case FT7:   return "FT7";
        case FT8:   return "FT8";
            // ... (Include all T, FT, TP cases) ...

            // --- Fallback for unhandled values (Good practice!) ---
        default:    return "UNKNOWN_LANDMARK";
        }
    }

    std::optional<Landmark> StringToLandmark(const std::string& str)
    {
        auto it = LandmarkMap.find(str);

        if (it != LandmarkMap.end()) {
            // Found the landmark! Return the enum value.
            return it->second;
        }
        else {
            // Landmark not found in the map.
            return std::nullopt; // Use return LANDMARK_NOT_FOUND; for pre-C++17
        }
    }

}