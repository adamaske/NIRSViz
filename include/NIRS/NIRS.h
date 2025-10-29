#pragma once
#include "Core/Base.h"

#include <optional> // For C++17 and later
#include <iostream>
#include <string>
#include <unordered_map>
#include <algorithm> // For std::transform (optional, but good for case insensitivity)
#include <map>

namespace NIRS {

    

    // --- Defintions ---
    static glm::vec4 SourceColor = glm::vec4(1.0f, 0.2f, 0.2f, 1.0f);
    static glm::vec4 DetectorColor = glm::vec4(0.2f, 0.2f, 1.0f, 1.0f);

    using ProbeID = uint32_t;
	using ChannelID = uint32_t;
    using ChannelDataID = uint32_t;
    using ChannelValue = double;

    struct Line {
        glm::vec3 Start;
        glm::vec3 End;
    };

    struct LineVertex {
        glm::vec3 Position;
        glm::vec4 Color;
    };

	// --- Wavelengths ---
    enum class WavelengthType {
        HBR = 0,
        HBO = 1,
        HBT = 2
    };
    static std::string WavelengthTypeToString(WavelengthType type) {
        switch (type) {
        case WavelengthType::HBR: return "HbR";
        case WavelengthType::HBO: return "HbO";
        case WavelengthType::HBT: return "HbT";
        }
        return "INVALID";
    }

	// --- Probes ---
    enum ProbeType {
        SOURCE,
        DETECTOR
    };

    struct Probe2D {
        glm::vec2 Position;
        ProbeType Type;
        ProbeID ID;
    };
    struct Probe3D {
        glm::vec3 Position;
        ProbeType Type;
        ProbeID ID;
    };

    struct Channel {
        ChannelID ID;

        ProbeID SourceID; // This is 1-indexed index
        ProbeID DetectorID;
        WavelengthType Wavelength;
        ChannelDataID DataIndex; // Index into the channel data registry
    };

    struct ChannelVisualization {
        ChannelID ChannelID;
        Line Line2D;
        Line Line3D;
        Line ProjectionLine3D;
		glm::vec3 IntersectionPoint3D; // Intersection point on cortex
    };

	// --- Projection ---
    struct ProjectionData {
        uint32_t HitDataTextureID;
        uint32_t NumHits;

        std::map<NIRS::ChannelID, glm::vec3> ChannelProjectionIntersections;
        std::map<NIRS::ChannelID, NIRS::ChannelValue> ChannelValues;
    };

    struct ProjectionSettings {
        float StrengthMin = -2.0005f;
        float StrengthMax = 2.0005f;
        float FalloffPower = 0.5f;
        float Radius = 1.6f;
        float DecayPower = 7.0f;
    };

    enum Landmark {
        // ----------------------------------------------------------------------
        // 1. ANATOMICAL LANDMARKS (Not strictly electrodes, but used for reference)
        Nz,     // Nasion
        Iz,     // Inion
        LPA,    // Left Pre-Auricular
        RPA,    // Right Pre-Auricular
        // ----------------------------------------------------------------------
        // 2. MIDLINE (Z) ELECTRODES (Antero-Posterior Sagittal)
        Fpz,    // Frontal Pole Zero
        AFz,    // Antero-Frontal Zero (10-10)
        Fz,     // Frontal Zero
        FCz,    // Fronto-Central Zero (10-10)
        Cz,     // Central Zero (Vertex)
        CPz,    // Centro-Parietal Zero (10-10)
        Pz,     // Parietal Zero
        POz,    // Parieto-Occipital Zero (10-10)
        Oz,     // Occipital Zero
        // ----------------------------------------------------------------------
        // 3. FRONTAL POLAR (Fp)
        Fp1, Fp2, Fp3, Fp4, Fp5, Fp6, Fp7, Fp8, Fp9, Fp10,
        FpA, FpAp, FpA_L, FpA_R, FpAFp, FpAFz, FpAFz_L, FpAFz_R,
        // ----------------------------------------------------------------------
        // 4. ANTERO-FRONTAL (AF)
        AFp1, AFp2, AFp3, AFp4, AFp5, AFp6, AFp7, AFp8, AFp9, AFp10,
        AF1, AF2, AF3, AF4, AF5, AF6, AF7, AF8, AF9, AF10,
        AFz_L, AFz_R, AFpAFp, AFpAFz, AFzAFp, AFzAFp_L, AFzAFp_R,
        // ----------------------------------------------------------------------
        // 5. FRONTAL (F)
        F1, F2, F3, F4, F5, F6, F7, F8, F9, F10,
        Fp1_L, Fp1_R, Fp2_L, Fp2_R, FpAFz_L_R, FpAFz_R_L,
        Fz_L, Fz_R, AFz_L_R, AFz_R_L, AFzAFp_L_R, AFzAFp_R_L,
        // ----------------------------------------------------------------------
        // 6. FRONTO-CENTRAL (FC)
        FC1, FC2, FC3, FC4, FC5, FC6, FC7, FC8,
        FC9, FC10, FCz_L, FCz_R,
        FCp, FCp_L, FCp_R, FCpAFz, FCpAFz_L, FCpAFz_R,
        FCC1, FCC2, FCC3, FCC4, FCC5, FCC6,
        FFC1, FFC2, FFC3, FFC4, FFC5, FFC6,
        // ----------------------------------------------------------------------
        // 7. CENTRAL (C)
        C1, C2, C3, C4, C5, C6,
        C7, C8, C9, C10, Cz_L, Cz_R,
        // ----------------------------------------------------------------------
        // 8. CENTRO-PARIETAL (CP)
        CP1, CP2, CP3, CP4, CP5, CP6,
        CP7, CP8, CP9, CP10, CPz_L, CPz_R,
        CPp, CPp_L, CPp_R, CPpAFz, CPpAFz_L, CPpAFz_R,
        CPP1, CPP2, CPP3, CPP4, CPP5, CPP6,
        CCP1, CCP2, CCP3, CCP4, CCP5, CCP6,
        // ----------------------------------------------------------------------
        // 9. PARIETAL (P)
        P1, P2, P3, P4, P5, P6, P7, P8, P9, P10,
        Pz_L, Pz_R,
        // ----------------------------------------------------------------------
        // 10. PARIETO-OCCIPITAL (PO)
        PO1, PO2, PO3, PO4, PO5, PO6, PO7, PO8, PO9, PO10,
        POz_L, POz_R,
        PPO1, PPO2, PPO3, PPO4, PPO5, PPO6,
        // ----------------------------------------------------------------------
        // 11. OCCIPITAL (O)
        O1, O2, O3, O4, O5, O6, O7, O8,
        Oz_L, Oz_R,
        // ----------------------------------------------------------------------
        // 12. TEMPORAL (T, FT, TP)
        // Temporal (The T-labels T3, T4, T5, T6 are generally replaced by T7, T8, P7, P8 
        // in the 10-10 system, but T9/T10 and their neighbors are part of the system)
        T3, T4, T5, T6, T7, T8, T9, T10,

        // Fronto-Temporal (FT)
        FT7, FT8, FT9, FT10,
        FTT7, FTT8, FTT9, FTT10,

        // Temporo-Parietal (TP)
        TP7, TP8, TP9, TP10,
        TPP7, TPP8, TPP9, TPP10,

        // Additional Temporal (A for Auricular/Mastoid)
        A1, A2,

        // ----------------------------------------------------------------------
        // 13. INTERMEDIATE/OTHER LOCATIONS (A large number of specific 10-5 locations)
        AFp, AFp_L, AFp_R, AFP1_L, AFP1_R, AFP2_L, AFP2_R,
        AFF1, AFF2, AFF3, AFF4, AFF5, AFF6,
        FCp_p, FCp_p_L, FCp_p_R,
        FCCp, FCCp_L, FCCp_R, FCCpAFz,
        CCP_p, CCP_p_L, CCP_p_R,
        CPP_p, CPP_p_L, CPP_p_R,
        PPO_p, PPO_p_L, PPO_p_R,
        POp, POp_L, POp_R,
        POp_p, POp_p_L, POp_p_R,

        // ----------------------------------------------------------------------
        // 14. INFERIOR (I) AND ORBITAL (OR)
        // These are often used as references or for EOG/EMG
        I1, I2,
        OR, OR_L, OR_R
        // ... and many more. The 10-5 system fully defines 345 locations.

        // The list above is a representative and functional subset containing 
        // all standard and common high-density names.
    };

    const std::unordered_map<std::string, Landmark> LandmarkMap = {
        // --- ANATOMICAL LANDMARKS ---
        {"Nz", Nz},
        {"Iz", Iz},
        {"LPA", LPA},
        {"RPA", RPA},

        // --- MIDLINE (Z) ELECTRODES ---
        {"Fpz", Fpz},
        {"AFz", AFz},
        {"Fz", Fz},
        {"FCz", FCz},
        {"Cz", Cz},
        {"CPz", CPz},
        {"Pz", Pz},
        {"POz", POz},
        {"Oz", Oz},

        // --- FRONTAL POLAR (Fp) ---
        {"Fp1", Fp1}, {"Fp2", Fp2}, {"F7", F7}, {"F8", F8}, {"FT7", FT7}, {"FT8", FT8}, /* ... all 10-5 Fp landmarks ... */

        // --- ANTERO-FRONTAL (AF) ---
        {"AF1", AF1}, {"AF2", AF2}, {"AF7", AF7}, {"AF8", AF8}, /* ... all 10-5 AF landmarks ... */

        // --- CENTRAL (C) ---
        {"C1", C1}, {"C2", C2}, {"C3", C3}, {"C4", C4}, {"C5", C5}, {"C6", C6}, /* ... all 10-5 C landmarks ... */

        // --- TEMPORAL (T) ---
        {"T3", T3}, {"T4", T4}, {"T5", T5},{"T6", T6}, {"T7", T7}, {"T8", T8},

		// --- PARIETAL (P) ---
		{"P1", P1}, {"P2", P2}, {"P3", P3}, {"P4", P4}, {"P5", P5}, {"P6", P6}, /* ... all 10-5 P landmarks ... */

        // --- OCCIPITAL (O) ---
		{"O1", O1}, {"O2", O2}, {"O3", O3}, {"O4", O4}, {"O5", O5}, {"O6", O6}, /* ... all 10-5 O landmarks ... */

    };

    std::string LandmarkToString(Landmark landmark);
    std::optional<Landmark> StringToLandmark(const std::string& str);
}