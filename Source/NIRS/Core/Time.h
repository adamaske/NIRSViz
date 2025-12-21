#pragma once
#include <filesystem>
#include <vector>

namespace NIRS {
    namespace Time {

        struct TimeData {
            std::vector<double> time;

            double duration = 0.0f;
            double sampling_frequency = 0.0f;
            // TODO : samplerate
        };

        inline TimeData LoadTimeData(std::filesystem::path filepath) {
            TimeData td;

            return td;
        }
    }
}