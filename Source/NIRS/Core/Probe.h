#pragma once

namespace NIRS {
	namespace Probe {

		using ChannelID = uint32_t; // UUID for channels
        using ChannelValue = double;
        using ChannelData = std::vector<ChannelValue>;
		using ChannelDataMap = std::map<ChannelID, ChannelData>;

        using OptodeID = uint32_t; // UUID for optodes

		using Position2D = glm::vec2;
        using Position3D = glm::vec3;

        enum OptodeType {
            SOURCE,
            DETECTOR,
			ELECTRODE // For EEG integration
        };

        struct Optode {
            OptodeType type;
            OptodeID id;

            Position2D position_2D;
            Position3D position_3D;
        };
        using OptodeMap = std::map<OptodeID, Optode>;

		struct Channel { // A channel is just a source-detector pair
            ChannelID id;

            OptodeID source_id; // This is 1-indexed index
            OptodeID detector_id;

            ChannelData hbo_data;
            ChannelData hbr_data;
            ChannelData hbt_data;
        };
        using ChannelMap = std::map<ChannelID, Channel>;

        struct Probe {
            ChannelMap channels;

            std::vector<Optode> optodes;

            OptodeMap sources;
			OptodeMap detectors;
        };

        //struct ProbeError {
        //    std::string message;
		//};

        namespace Utils {

            //bool ValidateProbe(const Probe& probe, std::vector<ProbeError>& out_errors) {
            //
            //    return true;
            //};

            constexpr std::string OptodeTypeToString(OptodeType type) {
                switch (type) {
                case SOURCE: return "SOURCE";
                case DETECTOR: return "DETECTOR";
				case ELECTRODE: return "ELECTRODE";
                }
            }

        }
	}
}
