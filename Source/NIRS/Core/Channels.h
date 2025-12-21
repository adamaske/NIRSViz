#pragma once
#include <vector>

#include "Probe.h"

namespace NIRS {
    namespace Channels {

        using ChannelValue = double;
        using ChannelDataVector = std::vector<ChannelValue>;


        struct Channel {
            // What is a channel ?
            // A channel has a source and detector
            NIRS::Probe::OptodeID source_id;
            NIRS::Probe::OptodeID detector_id;

            // A channel has some recordeded data
            ChannelDataVector data;
        };


        struct ChannelData {
            std::vector<Channel> channels;
        };

        inline ChannelData LoadChannelData(std::filesystem::path filepath) {
            ChannelData channel_data;


            return channel_data;
        }
    }
}
