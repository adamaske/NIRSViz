#include "pch.h"
#include "NIRS/SNIRFLoader.h"

#include <snirf/SNIRFFactory.h>

namespace NIRS {

bool LoadSNIRF(const std::filesystem::path& filepath,
               SNIRF& out,
               std::vector<SNIRFError>& out_errors)
{
    NVIZ_PROFILE_FUNCTION();

    SNIRFCPP::SNIRF raw;
    std::vector<SNIRFCPP::SNIRFError> raw_errors;

    if (!SNIRFCPP::SNIRFFactory::CreateSNIRF(raw, filepath, SNIRFCPP::UNKNOWN, raw_errors)) {
        for (auto& e : raw_errors)
            out_errors.push_back({ static_cast<SNIRFError::Code>(e.code), e.message });
        return false;
    }

    // ---- File info ----
    out.file_descriptor.filepath = raw.filepath;

    // ---- Metadata ----
    out.metadata.has_wings_generation = raw.metadata.has_wings_generation;
    for (auto& tag : raw.metadata.tags)
        out.metadata.tags.push_back({ tag.key, tag.value });

    // ---- Probe: sources ----
    for (auto& [id, opt] : raw.probe.sources) {
        Probe::Optode o;
        o.type        = Probe::SOURCE;
        o.id          = id;
        o.position_2D = glm::vec2(opt.position_2d[0], opt.position_2d[1]);
        o.position_3D = glm::vec3(opt.position_3d[0], opt.position_3d[1], opt.position_3d[2]);
        out.probe.sources[id] = o;
    }

    // ---- Probe: detectors ----
    for (auto& [id, opt] : raw.probe.detectors) {
        Probe::Optode o;
        o.type        = Probe::DETECTOR;
        o.id          = id;
        o.position_2D = glm::vec2(opt.position_2d[0], opt.position_2d[1]);
        o.position_3D = glm::vec3(opt.position_3d[0], opt.position_3d[1], opt.position_3d[2]);
        out.probe.detectors[id] = o;
    }

    // ---- Probe: wavelengths ----
    out.probe.wavelengths = raw.probe.wavelengths;

    // ---- Channels ----
    for (auto& [id, ch] : raw.probe.channels) {
        Probe::Channel channel;
        channel.id          = ch.id;
        channel.source_id   = ch.source_id;
        channel.detector_id = ch.detector_id;
        channel.hbo_data    = ch.hbo_data;
        channel.hbr_data    = ch.hbr_data;
        channel.hbt_data    = ch.hbt_data;
        out.probe.channels[id] = std::move(channel);
    }

    // ---- Time ----
    out.time_data.time               = raw.time_data.time;
    out.time_data.duration           = raw.time_data.duration;
    out.time_data.sampling_frequency = raw.time_data.sampling_frequency;

    // ---- Events ----
    for (auto& ev : raw.events.events) {
        Events::Event event;
        event.name = ev.name;
        for (auto& m : ev.markers)
            event.markers.push_back({ m.onset, m.duration, m.value });
        out.events.events.push_back(std::move(event));
    }

    // ---- Biosignals ----
    for (size_t i = 0; i < raw.biosignals.aux_data.size(); i++) {
        auto& aux = raw.biosignals.aux_data[i];
        Biosignals::AuxilaryData d;
        d.label = static_cast<Biosignals::AuxilaryLabel>(i);
        d.type  = static_cast<Biosignals::AuxlaryType>(aux.type);
        d.name  = aux.name;
        d.unit  = aux.unit;
        d.data  = aux.data;
        d.time  = aux.time;
        out.biosignals.aux_data.push_back(std::move(d));
    }

    NVIZ_INFO("SNIRF loaded: '{}' — {} sources, {} detectors, {} channels, {:.2f} Hz",
        filepath.filename().string(),
        out.GetSourceAmount(), out.GetDetectorAmount(),
        out.probe.channels.size(), out.GetSamplingRate());

    return true;
}

} // namespace NIRS
