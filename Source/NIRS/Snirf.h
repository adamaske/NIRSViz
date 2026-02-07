#pragma once

#include "NIRS/NIRS.h"

struct SNIRF {

    // ---- identity / origin ----
    NIRS::FileDescriptor file_descriptor;

    // ---- metadata tags ----
    NIRS::Metadata::Metadata metadata;

    // ---- probe geometry (sources, detectors, wavelengths) ----
    NIRS::Probe::Probe probe;

    // ---- per-channel hemodynamic time-series ----
    NIRS::Channels::ChannelDataStore channel_store;

    // ---- shared time axis ----
    NIRS::Time::TimeData time_data;

    // ---- stimulus / event markers ----
    NIRS::Events::EventsContainer events;

    // ---- auxiliary biosignals (respiration, GSR, PPG, …) ----
    NIRS::Biosignals::BiosignalData biosignals;

    // ---- convenience accessors (match old API surface) ----
    bool IsFileLoaded() const { return !file_descriptor.filepath.empty(); }
    std::string GetFilepath() const { return file_descriptor.filepath.string(); }

    const NIRS::Probe::Probe& GetProbe() const { return probe; }
    NIRS::Probe::Probe& GetProbe() { return probe; }

    int GetSourceAmount()   const { return static_cast<int>(probe.sources.size()); }
    int GetDetectorAmount() const { return static_cast<int>(probe.detectors.size()); }

    double GetSamplingRate()     const { return time_data.sampling_frequency; }
    double GetDurationSeconds()  const { return time_data.duration; }
    const std::vector<double>& GetTime() const { return time_data.time; }

    const std::vector<int>& GetWavelengths() const { return probe.wavelengths; }

    NIRS::Probe::ChannelMap GetChannels() const { return probe.channels; }

	// TODO : This method of checking is not good, we should have a more explicit way of checking if biosignals are present 
    // (e.g., a metadata tag) instead of relying on the presence of aux data, which could be empty even if biosignals are present.
    bool HasBiosignals() const { return metadata.has_wings_generation; }
};
