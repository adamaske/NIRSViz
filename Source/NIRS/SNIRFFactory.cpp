#include "pch.h"
#include "NIRS/SNIRFFactory.h"

#include "Core/Time.h"


SNIRF SNIRFFactory::CreateSNIRF(SNIRFType type, std::filesystem::path filepath) {
    // TODO : Filepath safety check


    // TODO : Implement type checker -> Are we able to parse it or not?


    //
    SNIRF snirf;
    snirf.channel_data = NIRS::Channels::LoadChannelData(filepath);

    snirf.time_data = NIRS::Time::LoadTimeData(filepath);

    snirf.optode_layout = NIRS::Probe::LoadOptodeLayout(filepath);

    snirf.events = NIRS::Events::LoadEvents(filepath);

    return snirf;
}
