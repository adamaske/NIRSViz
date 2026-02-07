#include "pch.h"
#include "NIRS/SNIRFLoader.h"

#include <HighFive/H5File.hpp>
#include <highfive/H5DataSet.hpp>
#include <highfive/H5DataSpace.hpp>
#include <highfive/H5Easy.hpp>

#include <Eigen/Dense>
#include <algorithm>
#include <cmath>

// ============================================================================
// HDF5 Helpers
// ============================================================================

namespace {
    using namespace HighFive;

    template <typename T>
    std::vector<T> ReadVector(const Group& group, const std::string& name) {
        try {
            DataSet dataset = group.getDataSet(name);
            std::vector<T> data;
            dataset.read(data);
            return data;
        }
        catch (const Exception& e) {
            NVIZ_ERROR("Failed to read dataset '{}/{}': {}", group.getPath(), name, e.what());
            return {};
        }
    }

    void DebugPrintGroup(const Group& group, const std::string& path) {
        for (const auto& name : group.listObjectNames()) {
            std::string current_path = path + "/" + name;
            ObjectType type = group.getObjectType(name);
            if (type == ObjectType::Group) {
                NVIZ_INFO("  [Group]   : {}", current_path);
                DebugPrintGroup(group.getGroup(name), current_path);
            }
            else if (type == ObjectType::Dataset) {
                NVIZ_INFO("  [Dataset] : {}", current_path);
            }
        }
    }
}

// ============================================================================
// Section Parsers  (each populates one part of the SNIRF struct)
// ============================================================================

namespace NIRS {

// --------------- Metadata ---------------------------------------------------

static void ParseMetadata(const HighFive::Group& metadata_group,
                          Metadata::Metadata& out)
{
    for (const auto& name : metadata_group.listObjectNames()) {
        NVIZ_INFO("Metadata tag: {}", name);

        if (name == "wingsGeneration") {
            out.has_wings_generation = true;     
        }

        // Store all tags as string key-value pairs where possible
        try {
            auto dataset = metadata_group.getDataSet(name);
            auto type_class = dataset.getDataType().getClass();
            if (type_class == HighFive::DataTypeClass::String) {
                std::string value;
                dataset.read(value);
                out.tags.push_back({ name, value });
            }
            else {
                out.tags.push_back({ name, "(non-string)" });
            }
        }
        catch (const HighFive::Exception&) {
            // Not a scalar dataset; skip
        }
    }
}

// --------------- Probe (sources, detectors, wavelengths) --------------------

static void ParseProbe(const HighFive::Group& probe_group,
                       Probe::Probe& out)
{
    using namespace Probe;
    using ChannelValue = double;

    // ---- Detectors ----
    {
        auto det2D = probe_group.getDataSet("detectorPos2D");
        auto det3D = probe_group.getDataSet("detectorPos3D");

        auto dims2D = det2D.getDimensions();
        auto dims3D = det3D.getDimensions();
        NVIZ_ASSERT(dims2D[0] == dims3D[0],
            "Detector count mismatch between 2D and 3D positions in SNIRF probe.");

        std::vector<ChannelValue> data2D(dims2D[0] * dims2D[1]);
        std::vector<ChannelValue> data3D(dims3D[0] * dims3D[1]);
        det2D.read_raw<ChannelValue>(data2D.data());
        det3D.read_raw<ChannelValue>(data3D.data());

        int n = static_cast<int>(dims2D[0]);
        for (int i = 0; i < n; i++) {
            Optode o;
            o.type = OptodeType::DETECTOR;
            o.id   = i + 1;
            o.position_2D = glm::vec2(data2D[i * 2 + 0], data2D[i * 2 + 1]);
            o.position_3D = glm::vec3(data3D[i * 3 + 0],
                                      data3D[i * 3 + 2],   // z-y swap (matches old code)
                                      data3D[i * 3 + 1]);
            out.detectors[o.id] = o;
        }
    }

    // ---- Sources ----
    {
        auto src2D = probe_group.getDataSet("sourcePos2D");
        auto src3D = probe_group.getDataSet("sourcePos3D");

        auto dims2D = src2D.getDimensions();
        auto dims3D = src3D.getDimensions();
        NVIZ_ASSERT(dims2D[0] == dims3D[0],
            "Source count mismatch between 2D and 3D positions in SNIRF probe.");

        std::vector<ChannelValue> data2D(dims2D[0] * dims2D[1]);
        std::vector<ChannelValue> data3D(dims3D[0] * dims3D[1]);
        src2D.read_raw<ChannelValue>(data2D.data());
        src3D.read_raw<ChannelValue>(data3D.data());

        int n = static_cast<int>(dims2D[0]);
        for (int i = 0; i < n; i++) {
            Optode o;
            o.type = OptodeType::SOURCE;
            o.id   = i + 1;
            o.position_2D = glm::vec2(data2D[i * 2 + 0], data2D[i * 2 + 1]);
            o.position_3D = glm::vec3(data3D[i * 3 + 0],
                                      data3D[i * 3 + 2],
                                      data3D[i * 3 + 1]);
            out.sources[o.id] = o;
        }
    }

    // ---- Wavelengths ----
    {
        auto wl_ds = probe_group.getDataSet("wavelengths");
        auto dims = wl_ds.getDimensions();
        std::vector<int> wl(dims[0]);
        wl_ds.read(wl);
        std::sort(wl.begin(), wl.end());
        out.wavelengths = std::move(wl);
    }
}

// --------------- Time -------------------------------------------------------

static void ParseTime(const HighFive::DataSet& time_ds,
                      Time::TimeData& out)
{
    std::vector<double> time_vec(time_ds.getDimensions()[0]);
    time_ds.read(time_vec);

    double total_duration = time_vec.back() - time_vec.front();
    size_t num_intervals  = time_vec.size() - 1;
    double avg_dt         = total_duration / static_cast<double>(num_intervals);

    out.time               = std::move(time_vec);
    out.duration           = total_duration;
    out.sampling_frequency = 1.0 / avg_dt;
}

// --------------- Channel Data (data1) ---------------------------------------

using EigenMapRM = Eigen::Map<const Eigen::Matrix<double,
                     Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>>;

static void ParseData1(const HighFive::Group& data1,
                       Time::TimeData& time_out,
                       Channels::ChannelDataStore& store_out,
                       Probe::Probe& probe_out)
{
    using namespace Probe;

    // ---- Time ----
    ParseTime(data1.getDataSet("time"), time_out);

    // ---- Raw matrix ----
    auto ts_ds = data1.getDataSet("dataTimeSeries");
    auto dims  = ts_ds.getDimensions();
    std::vector<double> raw(dims[0] * dims[1]);
    ts_ds.read_raw<double>(raw.data());
    // Transpose: HDF5 is [timepoints x channels] → we want [channels x timepoints]
    store_out.matrix = EigenMapRM(raw.data(), dims[0], dims[1]).transpose();

    auto& mat = store_out.matrix;

    NVIZ_ASSERT((mat.rows() % 2) == 0,
        "SNIRF data1 must have an even number of measurement rows (HbR + HbO pairs).");

    int half = static_cast<int>(mat.rows() / 2);

    // ---- Measurement lists → channels ----
    std::string base_name = "measurementList";
    for (int i = 0; i < half; i++) {
        int hbr_idx = i;
        int hbo_idx = i + half;

        auto ml = data1.getGroup(base_name + std::to_string(i + 1));

        int sourceIndex = 0, detectorIndex = 0;
        ml.getDataSet("sourceIndex").read(sourceIndex);
        ml.getDataSet("detectorIndex").read(detectorIndex);

        Channel ch;
        ch.id          = static_cast<ChannelID>(i);
        ch.source_id   = static_cast<OptodeID>(sourceIndex);
        ch.detector_id = static_cast<OptodeID>(detectorIndex);

        // HbR
        {
            auto row = mat.row(hbr_idx);
            ch.hbr_data.assign(row.data(), row.data() + row.size());
        }
        // HbO
        {
            auto row = mat.row(hbo_idx);
            ch.hbo_data.assign(row.data(), row.data() + row.size());
        }
        // HbT = HbO + HbR
        {
            ch.hbt_data.resize(ch.hbo_data.size());
            for (size_t t = 0; t < ch.hbo_data.size(); t++) {
                ch.hbt_data[t] = ch.hbo_data[t] + ch.hbr_data[t];
            }
        }

        probe_out.channels[ch.id] = std::move(ch);
    }
}

// --------------- Stims / Events ---------------------------------------------

static void ParseStims(const HighFive::Group& nirs,
                       Events::EventsContainer& out)
{
    for (size_t i = 1; i < 1000; i++) {
        std::string stim_name = "stim" + std::to_string(i);
        if (!nirs.exist(stim_name))
            break;

        auto stim = nirs.getGroup(stim_name);

        std::vector<std::vector<double>> data;
        try {
            stim.getDataSet("data").read(data);
        }
        catch (const HighFive::Exception& e) {
            NVIZ_ERROR("Failed to read 'data' for {}: {}", stim_name, e.what());
            continue;
        }

        Events::Event event;

        std::string name;
        stim.getDataSet("name").read<std::string>(name);
        event.name = name;

        event.markers.reserve(data.size());
        for (const auto& row : data) {
            if (row.size() < 3) {
                NVIZ_ERROR("Stim '{}' data row malformed ({} cols). Skipping.", name, row.size());
                continue;
            }
            event.markers.push_back({
                .onset    = row[0],
                .duration = row[1],
                .value    = row[2]
            });
        }

        std::sort(event.markers.begin(), event.markers.end(),
            [](const Events::EventMarker& a, const Events::EventMarker& b) {
                return a.onset < b.onset;
            });

        if (stim.exist("dataLabels")) {
            try {
                std::vector<std::string> labels;
                stim.getDataSet("dataLabels").read(labels);
                for (const auto& label : labels) {
                    NVIZ_INFO("    Stim label: {}", label);
                }
            }
            catch (const HighFive::Exception& e) {
                NVIZ_ERROR("Failed to read 'dataLabels' for {}: {}", stim_name, e.what());
            }
        }

        out.events.push_back(std::move(event));
    }
}

// --------------- Biosignals (aux) -------------------------------------------

static void ParseBiosignals(const HighFive::Group& nirs,
                            Biosignals::BiosignalData& out)
{
    using namespace Biosignals;

    // Map group name → label
    static const std::vector<std::pair<std::string, AuxilaryLabel>> aux_groups = {
        {"aux1", AUX1}, {"aux2", AUX2}, {"aux3", AUX3},
        {"aux4", AUX4}, {"aux5", AUX5}, {"aux6", AUX6},
        {"aux7", AUX7}, {"aux8", AUX8}, {"aux9", AUX9},
    };

    for (auto& [group_name, label] : aux_groups) {
        if (!nirs.exist(group_name))
            continue;

        NVIZ_DEBUG("Parsing biosignal group: {}", group_name);

        auto aux_group = nirs.getGroup(group_name);

        std::string name;
        aux_group.getDataSet("name").read(name);

        AuxlaryType type = UNKNOWN;
        if      (name == "Respiration") type = RESPIRATION;
        else if (name == "GSR")         type = GSR;
        else if (name == "Temperature") type = TEMPERATURE;
        else if (name == "ExGa_1")      type = EX_GA_1;
        else if (name == "ExGa_2")      type = EX_GA_2;
        else if (name == "ExGa_3")      type = EX_GA_3;
        else if (name == "PPG")         type = PPG;
        else if (name == "SpO2")        type = SP_O2;
        else if (name == "Heartrate")   type = HEARTRATE;

        std::string data_unit;
        aux_group.getDataSet("dataUnit").read(data_unit);

        std::vector<double> time_series;
        aux_group.getDataSet("dataTimeSeries").read(time_series);

        std::vector<double> time_vec;
        aux_group.getDataSet("time").read(time_vec);

        out.aux_data.push_back({
            .label = label,
            .type  = type,
            .name  = name,
            .unit  = data_unit,
            .data  = std::move(time_series),
            .time  = std::move(time_vec),
        });
    }
}

// ============================================================================
// Public API
// ============================================================================

bool LoadSNIRF(const std::filesystem::path& filepath,
               SNIRF& out,
               std::vector<SNIRFError>& out_errors)
{
    NVIZ_PROFILE_FUNCTION();

    if (!std::filesystem::exists(filepath)) {
        out_errors.push_back(SNIRFError(
            SNIRFError::FILE_NOT_FOUND,
            "File does not exist: " + filepath.string()));
        return false;
    }

    try {
        HighFive::File file(filepath.string(), HighFive::File::ReadOnly);

        // Debug dump
        //DebugPrintGroup(file.getGroup("/"), "");

        auto nirs = file.getGroup("/nirs");

        out.file_descriptor.filepath = filepath;

        // 1. Metadata
        if (nirs.exist("metaDataTags")) {
            ParseMetadata(nirs.getGroup("metaDataTags"), out.metadata);
        }

        // 2. Probe (must precede data1, channels reference sources/detectors)
        if (nirs.exist("probe")) {
            ParseProbe(nirs.getGroup("probe"), out.probe);
        }

        // 3. Data1 (time + channel data + measurement lists → probe.channels)
        if (nirs.exist("data1")) {
            ParseData1(nirs.getGroup("data1"),
                       out.time_data,
                       out.channel_store,
                       out.probe);
        }

        // 4. Stims / Events
        ParseStims(nirs, out.events);

        // 5. Biosignals (only if wingsGeneration metadata present)
        if (out.metadata.has_wings_generation) {
            ParseBiosignals(nirs, out.biosignals);
        }
    } catch (const HighFive::Exception& e) {
        out_errors.push_back(SNIRFError(
            SNIRFError::INVALID_FORMAT,
            "HDF5 parse error: " + std::string(e.what())));
        return false;
    }

    // Print summary
    NVIZ_INFO("SNIRF loaded    : {}", filepath.string());
    NVIZ_INFO("  Sample Rate   : {} Hz", out.time_data.sampling_frequency);
    NVIZ_INFO("  Sources       : {}", out.GetSourceAmount());
    NVIZ_INFO("  Detectors     : {}", out.GetDetectorAmount());
    NVIZ_INFO("  Channels      : {}", out.probe.channels.size());
    NVIZ_INFO("  Wavelengths   : {}", out.probe.wavelengths.size() >= 2
        ? std::to_string(out.probe.wavelengths[0]) + ", " + std::to_string(out.probe.wavelengths[1])
        : "N/A");
    NVIZ_INFO("  Events        : {}", out.events.events.size());
    NVIZ_INFO("  Biosignals    : {}", out.biosignals.aux_data.size());

    return true;
}

} // namespace NIRS
