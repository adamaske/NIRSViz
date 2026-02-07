#pragma once
#include <map>
#include <vector>

namespace NIRS {
    namespace Probe {

        using ChannelID    = uint32_t;
        using ChannelValue = double;
        using ChannelData  = std::vector<ChannelValue>;
        using ChannelDataMap = std::map<ChannelID, ChannelData>;

        using OptodeID = uint32_t;

        using Position2D = glm::vec2;
        using Position3D = glm::vec3;

        enum OptodeType {
            SOURCE,
            DETECTOR,
        };

        struct Optode {
            OptodeType type;
            OptodeID   id;

            Position2D position_2D;
            Position3D position_3D;
        };
        using OptodeMap = std::map<OptodeID, Optode>;

        struct Channel {
            ChannelID id;

            OptodeID source_id;   // 1-indexed
            OptodeID detector_id;

            ChannelData hbo_data;
            ChannelData hbr_data;
            ChannelData hbt_data;
        };
        using ChannelMap = std::map<ChannelID, Channel>;

        struct Probe {
            ChannelMap  channels;
            OptodeMap   sources;
            OptodeMap   detectors;

            std::vector<int> wavelengths;
        };

        struct OptodeLayout {
            std::vector<Optode> sources;
            std::vector<Optode> detectors;
        };

        namespace Utils {
            constexpr std::string OptodeTypeToString(OptodeType type) {
                switch (type) {
                case SOURCE:   return "SOURCE";
                case DETECTOR: return "DETECTOR";
                default:       return "UNKNOWN";
                }
            }
        }
    }
}
