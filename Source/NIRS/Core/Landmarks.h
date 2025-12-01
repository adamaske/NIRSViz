#pragma once
#include <string>
#include <unordered_map>
#include <algorithm>
#include <cctype>
#include <stdexcept>

namespace NIRS
{
    namespace Landmarks
    {

        enum Landmark {
            // ----------------------------------------------------------------------
            // 1. ANATOMICAL LANDMARKS (Not strictly electrodes, but used for reference)
            // ----------------------------------------------------------------------
            Nz,     // Nasion
            Iz,     // Inion
            LPA,    // Left Pre-Auricular
            RPA,    // Right Pre-Auricular

            // ----------------------------------------------------------------------
            // 2. MIDLINE (Z) ELECTRODES (Antero-Posterior Sagittal)
            // ----------------------------------------------------------------------
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
            // ----------------------------------------------------------------------
            Fp1, Fp2, Fp3, Fp4, Fp5, Fp6, Fp7, Fp8, Fp9, Fp10,
            FpA, FpAp, FpA_L, FpA_R, FpAFp, FpAFz, FpAFz_L, FpAFz_R,

            // ----------------------------------------------------------------------
            // 4. ANTERO-FRONTAL (AF)
            // ----------------------------------------------------------------------
            AFp1, AFp2, AFp3, AFp4, AFp5, AFp6, AFp7, AFp8, AFp9, AFp10,
            AF1, AF2, AF3, AF4, AF5, AF6, AF7, AF8, AF9, AF10,
            AFz_L, AFz_R, AFpAFp, AFpAFz, AFzAFp, AFzAFp_L, AFzAFp_R,

            // ----------------------------------------------------------------------
            // 5. FRONTAL (F)
            // ----------------------------------------------------------------------
            F1, F2, F3, F4, F5, F6, F7, F8, F9, F10,
            Fp1_L, Fp1_R, Fp2_L, Fp2_R, FpAFz_L_R, FpAFz_R_L,
            Fz_L, Fz_R, AFz_L_R, AFz_R_L, AFzAFp_L_R, AFzAFp_R_L,

            // ----------------------------------------------------------------------
            // 6. FRONTO-CENTRAL (FC)
            // ----------------------------------------------------------------------
            FC1, FC2, FC3, FC4, FC5, FC6, FC7, FC8, FC9, FC10,
            FCz_L, FCz_R,
            FCp, FCp_L, FCp_R, FCpAFz, FCpAFz_L, FCpAFz_R,
            FCC1, FCC2, FCC3, FCC4, FCC5, FCC6,
            FFC1, FFC2, FFC3, FFC4, FFC5, FFC6,

            // ----------------------------------------------------------------------
            // 7. CENTRAL (C)
            // ----------------------------------------------------------------------
            C1, C2, C3, C4, C5, C6, C7, C8, C9, C10,
            Cz_L, Cz_R,

            // ----------------------------------------------------------------------
            // 8. CENTRO-PARIETAL (CP)
            // ----------------------------------------------------------------------
            CP1, CP2, CP3, CP4, CP5, CP6, CP7, CP8, CP9, CP10,
            CPz_L, CPz_R,
            CPp, CPp_L, CPp_R, CPpAFz, CPpAFz_L, CPpAFz_R,
            CPP1, CPP2, CPP3, CPP4, CPP5, CPP6,
            CCP1, CCP2, CCP3, CCP4, CCP5, CCP6,

            // ----------------------------------------------------------------------
            // 9. PARIETAL (P)
            // ----------------------------------------------------------------------
            P1, P2, P3, P4, P5, P6, P7, P8, P9, P10,
            Pz_L, Pz_R,

            // ----------------------------------------------------------------------
            // 10. PARIETO-OCCIPITAL (PO)
            // ----------------------------------------------------------------------
            PO1, PO2, PO3, PO4, PO5, PO6, PO7, PO8, PO9, PO10,
            POz_L, POz_R,
            PPO1, PPO2, PPO3, PPO4, PPO5, PPO6,

            // ----------------------------------------------------------------------
            // 11. OCCIPITAL (O)
            // ----------------------------------------------------------------------
            O1, O2, O3, O4, O5, O6, O7, O8, O9, O10,
            Oz_L, Oz_R,

            // ----------------------------------------------------------------------
            // 12. TEMPORAL (T, FT, TP)
            // ----------------------------------------------------------------------
            // Temporal (T3, T4, T5, T6 are old 10-20 nomenclature)
            T3, T4, T5, T6, T7, T8, T9, T10,

            // Fronto-Temporal (FT)
            FT7, FT8, FT9, FT10,
            FTT7, FTT8, FTT9, FTT10,

            // Temporo-Parietal (TP)
            TP7, TP8, TP9, TP10,
            TPP7, TPP8, TPP9, TPP10,

            // Auricular/Mastoid references
            A1, A2,

            // ----------------------------------------------------------------------
            // 13. INTERMEDIATE/OTHER LOCATIONS (10-5 specific high-density positions)
            // ----------------------------------------------------------------------
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
            // ----------------------------------------------------------------------
            // Often used as references or for EOG/EMG in EEG, adapted for NIRS
            I1, I2,
            OR, OR_L, OR_R
        };

        // --- Static map for efficient and consistent bi-directional conversion ---

        // Helper to initialize the map
        static const std::unordered_map<Landmark, std::string>& GetLandmarkMap() {
            // Using a static local variable to ensure thread-safe lazy initialization (C++11+)
            static const std::unordered_map<Landmark, std::string> landmark_map = {
                {Nz, "Nz"}, {Iz, "Iz"}, {LPA, "LPA"}, {RPA, "RPA"},
                {Fpz, "Fpz"}, {AFz, "AFz"}, {Fz, "Fz"}, {FCz, "FCz"}, {Cz, "Cz"}, {CPz, "CPz"}, {Pz, "Pz"}, {POz, "POz"}, {Oz, "Oz"},

                {Fp1, "Fp1"}, {Fp2, "Fp2"}, {Fp3, "Fp3"}, {Fp4, "Fp4"}, {Fp5, "Fp5"}, {Fp6, "Fp6"}, {Fp7, "Fp7"}, {Fp8, "Fp8"}, {Fp9, "Fp9"}, {Fp10, "Fp10"},
                {FpA, "FpA"}, {FpAp, "FpAp"}, {FpA_L, "FpA_L"}, {FpA_R, "FpA_R"}, {FpAFp, "FpAFp"}, {FpAFz, "FpAFz"}, {FpAFz_L, "FpAFz_L"}, {FpAFz_R, "FpAFz_R"},

                {AFp1, "AFp1"}, {AFp2, "AFp2"}, {AFp3, "AFp3"}, {AFp4, "AFp4"}, {AFp5, "AFp5"}, {AFp6, "AFp6"}, {AFp7, "AFp7"}, {AFp8, "AFp8"}, {AFp9, "AFp9"}, {AFp10, "AFp10"},
                {AF1, "AF1"}, {AF2, "AF2"}, {AF3, "AF3"}, {AF4, "AF4"}, {AF5, "AF5"}, {AF6, "AF6"}, {AF7, "AF7"}, {AF8, "AF8"}, {AF9, "AF9"}, {AF10, "AF10"},
                {AFz_L, "AFz_L"}, {AFz_R, "AFz_R"}, {AFpAFp, "AFpAFp"}, {AFpAFz, "AFpAFz"}, {AFzAFp, "AFzAFp"}, {AFzAFp_L, "AFzAFp_L"}, {AFzAFp_R, "AFzAFp_R"},

                {F1, "F1"}, {F2, "F2"}, {F3, "F3"}, {F4, "F4"}, {F5, "F5"}, {F6, "F6"}, {F7, "F7"}, {F8, "F8"}, {F9, "F9"}, {F10, "F10"},
                {Fp1_L, "Fp1_L"}, {Fp1_R, "Fp1_R"}, {Fp2_L, "Fp2_L"}, {Fp2_R, "Fp2_R"}, {FpAFz_L_R, "FpAFz_L_R"}, {FpAFz_R_L, "FpAFz_R_L"},
                {Fz_L, "Fz_L"}, {Fz_R, "Fz_R"}, {AFz_L_R, "AFz_L_R"}, {AFz_R_L, "AFz_R_L"}, {AFzAFp_L_R, "AFzAFp_L_R"}, {AFzAFp_R_L, "AFzAFp_R_L"},

                {FC1, "FC1"}, {FC2, "FC2"}, {FC3, "FC3"}, {FC4, "FC4"}, {FC5, "FC5"}, {FC6, "FC6"}, {FC7, "FC7"}, {FC8, "FC8"}, {FC9, "FC9"}, {FC10, "FC10"},
                {FCz_L, "FCz_L"}, {FCz_R, "FCz_R"},
                {FCp, "FCp"}, {FCp_L, "FCp_L"}, {FCp_R, "FCp_R"}, {FCpAFz, "FCpAFz"}, {FCpAFz_L, "FCpAFz_L"}, {FCpAFz_R, "FCpAFz_R"},
                {FCC1, "FCC1"}, {FCC2, "FCC2"}, {FCC3, "FCC3"}, {FCC4, "FCC4"}, {FCC5, "FCC5"}, {FCC6, "FCC6"},
                {FFC1, "FFC1"}, {FFC2, "FFC2"}, {FFC3, "FFC3"}, {FFC4, "FFC4"}, {FFC5, "FFC5"}, {FFC6, "FFC6"},

                {C1, "C1"}, {C2, "C2"}, {C3, "C3"}, {C4, "C4"}, {C5, "C5"}, {C6, "C6"}, {C7, "C7"}, {C8, "C8"}, {C9, "C9"}, {C10, "C10"},
                {Cz_L, "Cz_L"}, {Cz_R, "Cz_R"},

                {CP1, "CP1"}, {CP2, "CP2"}, {CP3, "CP3"}, {CP4, "CP4"}, {CP5, "CP5"}, {CP6, "CP6"}, {CP7, "CP7"}, {CP8, "CP8"}, {CP9, "CP9"}, {CP10, "CP10"},
                {CPz_L, "CPz_L"}, {CPz_R, "CPz_R"},
                {CPp, "CPp"}, {CPp_L, "CPp_L"}, {CPp_R, "CPp_R"}, {CPpAFz, "CPpAFz"}, {CPpAFz_L, "CPpAFz_L"}, {CPpAFz_R, "CPpAFz_R"},
                {CPP1, "CPP1"}, {CPP2, "CPP2"}, {CPP3, "CPP3"}, {CPP4, "CPP4"}, {CPP5, "CPP5"}, {CPP6, "CPP6"},
                {CCP1, "CCP1"}, {CCP2, "CCP2"}, {CCP3, "CCP3"}, {CCP4, "CCP4"}, {CCP5, "CCP6"}, {CCP6, "CCP6"},

                {P1, "P1"}, {P2, "P2"}, {P3, "P3"}, {P4, "P4"}, {P5, "P5"}, {P6, "P6"}, {P7, "P7"}, {P8, "P8"}, {P9, "P9"}, {P10, "P10"},
                {Pz_L, "Pz_L"}, {Pz_R, "Pz_R"},

                {PO1, "PO1"}, {PO2, "PO2"}, {PO3, "PO3"}, {PO4, "PO4"}, {PO5, "PO5"}, {PO6, "PO6"}, {PO7, "PO7"}, {PO8, "PO8"}, {PO9, "PO9"}, {PO10, "PO10"},
                {POz_L, "POz_L"}, {POz_R, "POz_R"},
                {PPO1, "PPO1"}, {PPO2, "PPO2"}, {PPO3, "PPO3"}, {PPO4, "PPO4"}, {PPO5, "PPO5"}, {PPO6, "PPO6"},

                {O1, "O1"}, {O2, "O2"}, {O3, "O3"}, {O4, "O4"}, {O5, "O5"}, {O6, "O6"}, {O7, "O7"}, {O8, "O8"}, {O9, "O9"}, {O10, "O10"},
                {Oz_L, "Oz_L"}, {Oz_R, "Oz_R"},

                {T3, "T3"}, {T4, "T4"}, {T5, "T5"}, {T6, "T6"}, {T7, "T7"}, {T8, "T8"}, {T9, "T9"}, {T10, "T10"},
                {FT7, "FT7"}, {FT8, "FT8"}, {FT9, "FT9"}, {FT10, "FT10"},
                {FTT7, "FTT7"}, {FTT8, "FTT8"}, {FTT9, "FTT9"}, {FTT10, "FTT10"},
                {TP7, "TP7"}, {TP8, "TP8"}, {TP9, "TP9"}, {TP10, "TP10"},
                {TPP7, "TPP7"}, {TPP8, "TPP8"}, {TPP9, "TPP10"}, {TPP10, "TPP10"},
                {A1, "A1"}, {A2, "A2"},

                {AFp, "AFp"}, {AFp_L, "AFp_L"}, {AFp_R, "AFp_R"}, {AFP1_L, "AFP1_L"}, {AFP1_R, "AFP1_R"}, {AFP2_L, "AFP2_L"}, {AFP2_R, "AFP2_R"},
                {AFF1, "AFF1"}, {AFF2, "AFF2"}, {AFF3, "AFF3"}, {AFF4, "AFF4"}, {AFF5, "AFF5"}, {AFF6, "AFF6"},
                {FCp_p, "FCp_p"}, {FCp_p_L, "FCp_p_L"}, {FCp_p_R, "FCp_p_R"},
                {FCCp, "FCCp"}, {FCCp_L, "FCCp_L"}, {FCCp_R, "FCCp_R"}, {FCCpAFz, "FCCpAFz"},
                {CCP_p, "CCP_p"}, {CCP_p_L, "CCP_p_L"}, {CCP_p_R, "CCP_p_R"},
                {CPP_p, "CPP_p"}, {CPP_p_L, "CPP_p_L"}, {CPP_p_R, "CPP_p_R"},
                {PPO_p, "PPO_p"}, {PPO_p_L, "PPO_p_L"}, {PPO_p_R, "PPO_p_R"},
                {POp, "POp"}, {POp_L, "POp_L"}, {POp_R, "POp_R"},
                {POp_p, "POp_p"}, {POp_p_L, "POp_p_L"}, {POp_p_R, "POp_p_R"},

                {I1, "I1"}, {I2, "I2"},
                {OR, "OR"}, {OR_L, "OR_L"}, {OR_R, "OR_R"}
            };
            return landmark_map;
        }

        // --- Implementation of LandmarkToString ---

        // Uses a constexpr return type (string_view in C++17/20 or the mapping approach below)
        // Since the prompt uses std::string as a return type, a robust lookup approach is preferred.
        // We use the map approach and return the string by value/const ref (depending on caller's requirements).
        // Since the prompt specified `constexpr std::string`, we'll return a copy of the string from the map.
        static const std::string LandmarkToString(Landmark landmark) {
            auto const& map = GetLandmarkMap();
            // In a real C++ project, you might consider std::string_view for efficiency
            auto it = map.find(landmark);
            if (it != map.end()) {
                // Returns a copy of the string name
                return it->second;
            }
            // Handle unknown/out-of-range enum values
            throw std::out_of_range("Unknown Landmark value.");
        }

        // --- Implementation of StringToLandmark ---

        // Helper to convert a string to uppercase for case-insensitive lookup
        inline std::string ToUpper(const std::string& str) {
            std::string upper_str = str;
            std::transform(upper_str.begin(), upper_str.end(), upper_str.begin(),
                [](unsigned char c) { return std::toupper(c); });
            return upper_str;
        }

        // Helper to create the reverse map (string name -> enum value)
        static const std::unordered_map<std::string, Landmark>& GetLandmarkReverseMap() {
            static const std::unordered_map<std::string, Landmark> reverse_map = [] {
                std::unordered_map<std::string, Landmark> r_map;
                for (const auto& pair : GetLandmarkMap()) {
                    // Store the string in uppercase for case-insensitive lookup
                    r_map[ToUpper(pair.second)] = pair.first;
                }
                return r_map;
                }();
            return reverse_map;
        }


        static Landmark StringToLandmark(const std::string& landmark_name) {
            // Convert input string to uppercase for case-insensitive comparison
            std::string upper_name = ToUpper(landmark_name);

            auto const& reverse_map = GetLandmarkReverseMap();
            auto it = reverse_map.find(upper_name);

            if (it != reverse_map.end()) {
                // Found the mapping
                return it->second;
            }

            // Handle unknown landmark names
			NVIZ_ERROR("Unknown landmark name: {}", landmark_name);
            throw std::out_of_range("Unknown landmark name: " + landmark_name);
        }
    }
}