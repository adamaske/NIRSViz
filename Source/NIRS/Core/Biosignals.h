#pragma once

#include <string>
#include <vector>
#include <map>

namespace NIRS {
    namespace Biosignals {

        enum AuxilaryLabel {
            AUX1, AUX2, AUX3, AUX4, AUX5,
            AUX6, AUX7, AUX8, AUX9,
        };

        enum AuxlaryType {
            UNKNOWN,
            RESPIRATION,
            GSR,
            TEMPERATURE,
            EX_GA_1,
            EX_GA_2,
            EX_GA_3,
            PPG,
            SP_O2,
            HEARTRATE
        };

        struct AuxilaryData {
            AuxilaryLabel label;
            AuxlaryType   type;

            std::string name;
            std::string unit;

            std::vector<double> data;
            std::vector<double> time;
        };

        struct BiosignalData {
            std::vector<AuxilaryData> aux_data = {};
        };
    }
}
