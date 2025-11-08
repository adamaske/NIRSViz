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
        case Fp3:   return "Fp3";
        case Fp4:   return "Fp4";
        case Fp5:   return "Fp5";
        case Fp6:   return "Fp6";
        case Fp7:   return "Fp7";
        case Fp8:   return "Fp8";
        case Fp9:   return "Fp9";
        case Fp10:  return "Fp10";
        case FpA:   return "FpA";
        case FpAp:  return "FpAp";
        case FpA_L: return "FpA_L";
        case FpA_R: return "FpA_R";
        case FpAFp: return "FpAFp";
        case FpAFz: return "FpAFz";
        case FpAFz_L: return "FpAFz_L";
        case FpAFz_R: return "FpAFz_R";

            // --- ANTERO-FRONTAL (AF) ---
        case AFp1:  return "AFp1";
        case AFp2:  return "AFp2";
        case AFp3:  return "AFp3";
        case AFp4:  return "AFp4";
        case AFp5:  return "AFp5";
        case AFp6:  return "AFp6";
        case AFp7:  return "AFp7";
        case AFp8:  return "AFp8";
        case AFp9:  return "AFp9";
        case AFp10: return "AFp10";
        case AF1:   return "AF1";
        case AF2:   return "AF2";
        case AF3:   return "AF3";
        case AF4:   return "AF4";
        case AF5:   return "AF5";
        case AF6:   return "AF6";
        case AF7:   return "AF7";
        case AF8:   return "AF8";
        case AF9:   return "AF9";
        case AF10:  return "AF10";
        case AFz_L: return "AFz_L";
        case AFz_R: return "AFz_R";
        case AFpAFp: return "AFpAFp";
        case AFpAFz: return "AFpAFz";
        case AFzAFp: return "AFzAFp";
        case AFzAFp_L: return "AFzAFp_L";
        case AFzAFp_R: return "AFzAFp_R";

            // --- FRONTAL (F) ---
        case F1:    return "F1";
        case F2:    return "F2";
        case F3:    return "F3";
        case F4:    return "F4";
        case F5:    return "F5";
        case F6:    return "F6";
        case F7:    return "F7";
        case F8:    return "F8";
        case F9:    return "F9";
        case F10:   return "F10";
        case Fp1_L: return "Fp1_L";
        case Fp1_R: return "Fp1_R";
        case Fp2_L: return "Fp2_L";
        case Fp2_R: return "Fp2_R";
        case FpAFz_L_R: return "FpAFz_L_R";
        case FpAFz_R_L: return "FpAFz_R_L";
        case Fz_L:  return "Fz_L";
        case Fz_R:  return "Fz_R";
        case AFz_L_R: return "AFz_L_R";
        case AFz_R_L: return "AFz_R_L";
        case AFzAFp_L_R: return "AFzAFp_L_R";
        case AFzAFp_R_L: return "AFzAFp_R_L";

            // --- FRONTO-CENTRAL (FC) ---
        case FC1:   return "FC1";
        case FC2:   return "FC2";
        case FC3:   return "FC3";
        case FC4:   return "FC4";
        case FC5:   return "FC5";
        case FC6:   return "FC6";
        case FC7:   return "FC7";
        case FC8:   return "FC8";
        case FC9:   return "FC9";
        case FC10:  return "FC10";
        case FCz_L: return "FCz_L";
        case FCz_R: return "FCz_R";
        case FCp:   return "FCp";
        case FCp_L: return "FCp_L";
        case FCp_R: return "FCp_R";
        case FCpAFz: return "FCpAFz";
        case FCpAFz_L: return "FCpAFz_L";
        case FCpAFz_R: return "FCpAFz_R";
        case FCC1:  return "FCC1";
        case FCC2:  return "FCC2";
        case FCC3:  return "FCC3";
        case FCC4:  return "FCC4";
        case FCC5:  return "FCC5";
        case FCC6:  return "FCC6";
        case FFC1:  return "FFC1";
        case FFC2:  return "FFC2";
        case FFC3:  return "FFC3";
        case FFC4:  return "FFC4";
        case FFC5:  return "FFC5";
        case FFC6:  return "FFC6";

            // --- CENTRAL (C) ---
        case C1:    return "C1";
        case C2:    return "C2";
        case C3:    return "C3";
        case C4:    return "C4";
        case C5:    return "C5";
        case C6:    return "C6";
        case C7:    return "C7";
        case C8:    return "C8";
        case C9:    return "C9";
        case C10:   return "C10";
        case Cz_L:  return "Cz_L";
        case Cz_R:  return "Cz_R";

            // --- CENTRO-PARIETAL (CP) ---
        case CP1:   return "CP1";
        case CP2:   return "CP2";
        case CP3:   return "CP3";
        case CP4:   return "CP4";
        case CP5:   return "CP5";
        case CP6:   return "CP6";
        case CP7:   return "CP7";
        case CP8:   return "CP8";
        case CP9:   return "CP9";
        case CP10:  return "CP10";
        case CPz_L: return "CPz_L";
        case CPz_R: return "CPz_R";
        case CPp:   return "CPp";
        case CPp_L: return "CPp_L";
        case CPp_R: return "CPp_R";
        case CPpAFz: return "CPpAFz";
        case CPpAFz_L: return "CPpAFz_L";
        case CPpAFz_R: return "CPpAFz_R";
        case CPP1:  return "CPP1";
        case CPP2:  return "CPP2";
        case CPP3:  return "CPP3";
        case CPP4:  return "CPP4";
        case CPP5:  return "CPP5";
        case CPP6:  return "CPP6";
        case CCP1:  return "CCP1";
        case CCP2:  return "CCP2";
        case CCP3:  return "CCP3";
        case CCP4:  return "CCP4";
        case CCP5:  return "CCP5";
        case CCP6:  return "CCP6";

            // --- PARIETAL (P) ---
        case P1:    return "P1";
        case P2:    return "P2";
        case P3:    return "P3";
        case P4:    return "P4";
        case P5:    return "P5";
        case P6:    return "P6";
        case P7:    return "P7";
        case P8:    return "P8";
        case P9:    return "P9";
        case P10:   return "P10";
        case Pz_L:  return "Pz_L";
        case Pz_R:  return "Pz_R";

            // --- PARIETO-OCCIPITAL (PO) ---
        case PO1:   return "PO1";
        case PO2:   return "PO2";
        case PO3:   return "PO3";
        case PO4:   return "PO4";
        case PO5:   return "PO5";
        case PO6:   return "PO6";
        case PO7:   return "PO7";
        case PO8:   return "PO8";
        case PO9:   return "PO9";
        case PO10:  return "PO10";
        case POz_L: return "POz_L";
        case POz_R: return "POz_R";
        case PPO1:  return "PPO1";
        case PPO2:  return "PPO2";
        case PPO3:  return "PPO3";
        case PPO4:  return "PPO4";
        case PPO5:  return "PPO5";
        case PPO6:  return "PPO6";

            // --- OCCIPITAL (O) ---
        case O1:    return "O1";
        case O2:    return "O2";
        case O3:    return "O3";
        case O4:    return "O4";
        case O5:    return "O5";
        case O6:    return "O6";
        case O7:    return "O7";
        case O8:    return "O8";
        case O9:    return "O9";
        case O10:   return "O10";
        case Oz_L:  return "Oz_L";
        case Oz_R:  return "Oz_R";

            // --- TEMPORAL (T, FT, TP) ---
        case T3:    return "T3";
        case T4:    return "T4";
        case T5:    return "T5";
        case T6:    return "T6";
        case T7:    return "T7";
        case T8:    return "T8";
        case T9:    return "T9";
        case T10:   return "T10";

            // Fronto-Temporal (FT)
        case FT7:   return "FT7";
        case FT8:   return "FT8";
        case FT9:   return "FT9";
        case FT10:  return "FT10";
        case FTT7:  return "FTT7";
        case FTT8:  return "FTT8";
        case FTT9:  return "FTT9";
        case FTT10: return "FTT10";

            // Temporo-Parietal (TP)
        case TP7:   return "TP7";
        case TP8:   return "TP8";
        case TP9:   return "TP9";
        case TP10:  return "TP10";
        case TPP7:  return "TPP7";
        case TPP8:  return "TPP8";
        case TPP9:  return "TPP9";
        case TPP10: return "TPP10";

            // Auricular/Mastoid
        case A1:    return "A1";
        case A2:    return "A2";

            // --- INTERMEDIATE/OTHER LOCATIONS (10-5 specific) ---
        case AFp:   return "AFp";
        case AFp_L: return "AFp_L";
        case AFp_R: return "AFp_R";
        case AFP1_L: return "AFP1_L";
        case AFP1_R: return "AFP1_R";
        case AFP2_L: return "AFP2_L";
        case AFP2_R: return "AFP2_R";
        case AFF1:  return "AFF1";
        case AFF2:  return "AFF2";
        case AFF3:  return "AFF3";
        case AFF4:  return "AFF4";
        case AFF5:  return "AFF5";
        case AFF6:  return "AFF6";
        case FCp_p: return "FCp_p";
        case FCp_p_L: return "FCp_p_L";
        case FCp_p_R: return "FCp_p_R";
        case FCCp:  return "FCCp";
        case FCCp_L: return "FCCp_L";
        case FCCp_R: return "FCCp_R";
        case FCCpAFz: return "FCCpAFz";
        case CCP_p: return "CCP_p";
        case CCP_p_L: return "CCP_p_L";
        case CCP_p_R: return "CCP_p_R";
        case CPP_p: return "CPP_p";
        case CPP_p_L: return "CPP_p_L";
        case CPP_p_R: return "CPP_p_R";
        case PPO_p: return "PPO_p";
        case PPO_p_L: return "PPO_p_L";
        case PPO_p_R: return "PPO_p_R";
        case POp:   return "POp";
        case POp_L: return "POp_L";
        case POp_R: return "POp_R";
        case POp_p: return "POp_p";
        case POp_p_L: return "POp_p_L";
        case POp_p_R: return "POp_p_R";

            // --- INFERIOR (I) AND ORBITAL (OR) ---
        case I1:    return "I1";
        case I2:    return "I2";
        case OR:    return "OR";
        case OR_L:  return "OR_L";
        case OR_R:  return "OR_R";

            // --- Fallback for unhandled values ---
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