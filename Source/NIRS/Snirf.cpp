#include "pch.h"
#include "NIRS/Snirf.h"

#include <HighFive/H5File.hpp>
#include <highfive/H5DataSet.hpp>
#include <highfive/H5DataSpace.hpp>
#include <highfive/H5Easy.hpp>

#include <NIRS/SNIRFValidator.h>

using namespace HighFive;
using namespace NIRS;

namespace Utils {
    using namespace HighFive;
    using namespace NIRS;

    

    template <typename T>
    std::vector<T> read_vector(const Group& group, const std::string& name) {
        try {
            DataSet dataset = group.getDataSet(name);
            std::vector<T> data;
            dataset.read(data);
            return data;
        }
        catch (const Exception& e) {
            NVIZ_ERROR("Failed to read 1D Dataset '{0}': {1}", group.getPath() + "/" + name, e.what());
            return {};
        }
    }
    template <typename T>
    std::vector<T> read_2d_flat_vector(const Group& group, const std::string& name) {
        try {
            DataSet dataset = group.getDataSet(name);
            std::vector<std::vector<T>> data_2d;
            dataset.read(data_2d);

            // Flatten the 2D vector into a 1D vector (for easier GPU upload/handling)
            std::vector<T> data_flat;
            if (!data_2d.empty()) {
                size_t total_size = data_2d.size() * data_2d[0].size();
                data_flat.reserve(total_size);
                for (const auto& row : data_2d) {
                    data_flat.insert(data_flat.end(), row.begin(), row.end());
                }
            }
            return data_flat;
        }
        catch (const Exception& e) {
            NVIZ_ERROR("Failed to read 2D Dataset '{0}': {1}", group.getPath() + "/" + name, e.what());
            return {};
        }
    }
    std::string get_dataset_shape(const DataSet& dataset){
        std::vector<size_t> dims = dataset.getDimensions();
        std::string shape_str = "(";
        for (size_t i = 0; i < dims.size(); ++i) {
            shape_str += std::to_string(dims[i]);
            if (i < dims.size() - 1) {
                shape_str += ", ";
            }
        }
        shape_str += ")";
        return shape_str;
    }
    // Recursive function to traverse the HDF5 structure
    void ParseGroup(const Group& current_group, const std::string& path) {
        // 1. Get all object names in the current group
        std::vector<std::string> object_names = current_group.listObjectNames();

        // 2. Iterate through objects
        for (const auto& name : object_names) {
            std::string current_path = path + "/" + name;
            ObjectType type = current_group.getObjectType(name);

            switch (type) {
            case ObjectType::Group: {
                // It's a Group: print info and recurse
                NVIZ_INFO("  [Group]  : {0}", current_path);
                Group next_group = current_group.getGroup(name);
                ParseGroup(next_group, current_path);
                break;
            }
            case ObjectType::Dataset: {
                // It's a Dataset: print info including shape
                DataSet dataset = current_group.getDataSet(name);
                std::string shape = get_dataset_shape(dataset);
                //NVIZ_INFO("  [Dataset] : {0}, {Shape] : {1}", current_path.c_str(), shape.c_str());
                break;
            }
            default: {
                // Other types (e.g., named DataType)
                NVIZ_INFO("  [Other] : {0}", current_path);
                break;
            }
            }
        }
    }

    // Main parsing function
    File ParseHDF5(const std::string& filepath) {
        // Open the file in read-only mode
        File file(filepath, File::ReadOnly);

        NVIZ_INFO("Parsing HDF5 : {0}", filepath);

        // Start the recursive parsing from the root group (which is the File object itself)
        Group root_group = file.getGroup("/");
        ParseGroup(root_group, "");

        NVIZ_INFO("END OF FILE : {0}", filepath);

        return file;
    }
}

SNIRF::SNIRF(const std::filesystem::path& filepath) : filepath_(filepath)
{
    if (!std::filesystem::exists(filepath)) {
        NVIZ_ERROR("SNIRF file does not exist: {0}", filepath.string().c_str());
        return;
    }
 
    try {
        LoadFile(filepath);
    }
    catch (const HighFive::Exception& e) {
        NVIZ_ERROR("SNIRF : Failed to load file {0} : {1}", filepath.string(), e.what());
	}
}

void SNIRF::Print()
{
    NVIZ_INFO("SNIRF File       : {}", filepath_.string());
	NVIZ_INFO("Sample Rate : {} Hz", m_SamplingRate);
    NVIZ_INFO("     Sources     : {}", GetSourceAmount());
    NVIZ_INFO("     Detectors   : {}", GetDetectorAmount());

    //NVIZ_INFO("Landmarks : 3D{}", m_Landmarks.size());
    //auto print_count = std::min((size_t)3, m_Landmarks.size());
    //for (size_t i = 0; i < print_count; i++)
    //{
    //    auto& lm = m_Landmarks[i];
    //    NVIZ_INFO("    {} : ( {}, {}, {} )", lm.Name, lm.Position.x, lm.Position.y, lm.Position.z);
    //}

    NVIZ_INFO("Wavelengths : {}, {}", m_Wavelengths[0], m_Wavelengths[1]);

    NVIZ_INFO("Channel Data : {} channels, {} time points", m_ChannelData.rows(), m_ChannelData.cols());

    // Print Events
    for(auto& event : events_)
    {
		NVIZ_INFO("Event : {}", event.name);

        int i = 0;
        for (auto& marker : event.markers)
        {
			NVIZ_INFO("    {} : Onset: {}, Duration: {}, Value: {}", i++, marker.onset, marker.duration, marker.value);
        }
	}
}

void SNIRF::LoadFile(const std::filesystem::path& filepath)
{
    File file(filepath.string(), File::ReadOnly); 
	//Utils::ParseHDF5(filepath.string());

	Group root = file.getGroup("/");
    Group nirs = root.getGroup("nirs");

    ParseMetadataTags(nirs.getGroup("metaDataTags"));
	ParseProbe(nirs.getGroup("probe")); // THIS MUST BE FIRST
    ParseData1(nirs.getGroup("data1"));
    ParseStims(nirs);

    Print();
}


void SNIRF::ParseMetadataTags(const HighFive::Group& metadata)
{

    std::vector<std::string> object_names = metadata.listObjectNames();
    for(auto& name : object_names)
    {
        DataSet dataset = metadata.getDataSet(name);
        NVIZ_INFO("Metadata Tag : {0}", name);
	}
}

void SNIRF::ParseProbe(const HighFive::Group& probe)
{
    using namespace NIRS::Probe;

    std::vector<std::string> object_names = probe.listObjectNames();

    using Map_RM = Eigen::Map<const Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>>;



    OptodeID source2D = 0;
    OptodeID source3D = 0;
    
    OptodeID detector2D = 0;
    OptodeID detector3D = 0;

    {
        auto detectors_2D = probe.getDataSet("detectorPos2D");
        auto detectors_3D = probe.getDataSet("detectorPos3D");


        auto dims_2D = detectors_2D.getDimensions();
        auto dims_3D = detectors_3D.getDimensions();

        NVIZ_ASSERT(dims_2D[0] == dims_3D[0], "Source count mismatch between 2D and 3D positions in SNIRF probe.");

		std::vector<ChannelValue> data_2D(dims_2D[0] * dims_2D[1]);
		std::vector<ChannelValue> data_3D(dims_3D[0] * dims_3D[1]);

		detectors_2D.read_raw<ChannelValue>(data_2D.data());
		detectors_3D.read_raw<ChannelValue>(data_3D.data());

        int num_detectors = dims_2D[0];
        for (int i = 0; i < num_detectors; i++)
        {
            double x2D = data_2D[i * 2 + 0];
            double y2D = data_2D[i * 2 + 1];

            double x3D = data_3D[i * 3 + 0];
            double y3D = data_3D[i * 3 + 1];
            double z3D = data_3D[i * 3 + 2];

            Optode optode;
            optode.type = OptodeType::DETECTOR;
            optode.id = i + 1;

            optode.position_2D = glm::vec2(x2D, y2D);
            optode.position_3D = glm::vec3(x3D, y3D, z3D);

			probe_.detectors[optode.id] = optode;
        }
    }

    {
		auto sources_2D = probe.getDataSet("sourcePos2D");
		auto sources_3D = probe.getDataSet("sourcePos3D");

		auto dims_2D = sources_2D.getDimensions();
		auto dims_3D = sources_3D.getDimensions();

		NVIZ_ASSERT(dims_2D[0] == dims_3D[0], "Source count mismatch between 2D and 3D positions in SNIRF probe.");

		std::vector<ChannelValue> data_2D(dims_2D[0] * dims_2D[1]);
		std::vector<ChannelValue> data_3D(dims_3D[0] * dims_3D[1]);
        
		sources_2D.read_raw<ChannelValue>(data_2D.data());
		sources_3D.read_raw<ChannelValue>(data_3D.data());

		// Now populate the probe_ structure

		int num_sources = dims_2D[0];
        for (int i = 0; i < num_sources; i++)
        {
			double x2D = data_2D[(i * 2) + 0];
			double y2D = data_2D[(i * 2) + 1];

			double x3D = data_3D[(i * 3) + 0];
			double y3D = data_3D[(i * 3) + 1];
			double z3D = data_3D[(i * 3) + 2];

            Optode optode;
			optode.type = OptodeType::SOURCE;
            optode.id = i + 1;

			optode.position_2D = glm::vec2(x2D, y2D);
			optode.position_3D = glm::vec3(x3D, y3D, z3D);

			probe_.sources[optode.id] = optode;
        }
    }

    auto wavelengths = probe.getDataSet("wavelengths");
    {
        auto dims = wavelengths.getDimensions();
        std::vector<int> wl(dims[0]);
		wavelengths.read(wl);
		m_Wavelengths = wl; 
        std::sort(m_Wavelengths.begin(), m_Wavelengths.end()); // Sort in ascending order to make sure HbR is the 0th 
    }

    //auto landmarkLabels = probe.getDataSet("landmarkLabels");
    //auto landmarkPos3D = probe.getDataSet("landmarkPos3D");
    //{
    //    auto label_dims = landmarkLabels.getDimensions();
    //    std::vector<std::string> labels(label_dims[0]);
    //    landmarkLabels.read(labels);
    //
    //    auto dims = landmarkPos3D.getDimensions();
    //    auto nd_array = std::vector<double>(dims[0] * dims[1]);
    //    landmarkPos3D.read_raw<double>(nd_array.data());
    //    auto positions = Map_RM(nd_array.data(), dims[0], dims[1]);
    //
    //    for (int i = 0; i < positions.rows(); i++) {
    //
    //        auto row_vector = positions.row(i);
    //
    //        // Process the data for the i-th detector
    //        double x = row_vector(0);
    //        double y = row_vector(1);
    //        double z = row_vector(2);
    //
    //        m_Landmarks.push_back({ labels[i], { x, y, z} });
    //    }
    //}
}

void SNIRF::ParseData1(const HighFive::Group& data1)
{
	using namespace NIRS::Probe;

    DataSet time = data1.getDataSet("time");
    {
        std::vector<double> time_data(time.getDimensions()[0]);
        time.read(time_data);

        m_Time.resize(time_data.size());
		std::copy(time_data.begin(), time_data.end(), m_Time.begin());

        float total_duration = time_data.back() - time_data.front();
        size_t num_intervals = time_data.size() - 1;
        float avg_dt = total_duration / num_intervals;
        float sampling_rate = 1.0f / avg_dt;

        m_SamplingRate = sampling_rate;
		m_DurationSeconds = total_duration;
    }

    using Map_RM = Eigen::Map<const Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>>;
    auto dataTimeSeries = data1.getDataSet("dataTimeSeries");
    {
        auto dims = dataTimeSeries.getDimensions();
        auto nd_array = std::vector<double>(dims[0] * dims[1]);
        dataTimeSeries.read_raw<double>(nd_array.data());
        m_ChannelData = Map_RM(nd_array.data(), dims[0], dims[1]).transpose();
	}

	// The first half is hbr, the second half is hbo
    NVIZ_ASSERT((m_ChannelData.rows() % 2) == 0, "Snirf file must have even amount of channels");

	std::string base_name = "measurementList";
    for (size_t i = 0; i < m_ChannelData.rows() / 2; i++)
    {
        int hbr_index = i;
		int hbo_index = hbr_index + (m_ChannelData.rows() / 2);

		auto name = base_name + std::to_string(i+1); // measurementList is 1 indexed

		auto measurementList = data1.getGroup(name);

        auto dataType = 0;
        measurementList.getDataSet("dataType").read(dataType);

		auto dataTypeIndex = 0;
        measurementList.getDataSet("dataTypeIndex").read(dataTypeIndex);

        std::string dataTypeLabel = "";
        measurementList.getDataSet("dataTypeLabel").read(dataTypeLabel);

        int sourceIndex = 0;
        measurementList.getDataSet("sourceIndex").read(sourceIndex);

        int detectorIndex = 0;
        measurementList.getDataSet("detectorIndex").read(detectorIndex);

        int wavelengthIndex = 0;
        measurementList.getDataSet("wavelengthIndex").read(wavelengthIndex);
        
        Channel channel;
		channel.id = i; // As long as its unique this should be fine
		channel.source_id = sourceIndex; // These are 1-indexed, TODO : Fix 
		channel.detector_id = detectorIndex;
       
        { // Load HBR
            auto channel_row = m_ChannelData.row(hbr_index);
            std::vector<double> channel_data_vec(channel_row.size());
            std::copy(channel_row.data(), channel_row.data() + channel_row.size(), channel_data_vec.begin());

            std::vector<double> processed;
            //PreprocessHemodynamicData(channel_data_vec, processed, m_SamplingRate);

            channel.hbr_data = channel_data_vec;
        };
        
        { // Load HBO
            auto channel_row = m_ChannelData.row(hbo_index);
            std::vector<double> channel_data_vec(channel_row.size());
            std::copy(channel_row.data(), channel_row.data() + channel_row.size(), channel_data_vec.begin());

            std::vector<double> processed;
            //PreprocessHemodynamicData(channel_data_vec, processed, m_SamplingRate);

            channel.hbo_data = channel_data_vec;
        };

        probe_.channels[channel.id] = channel;
    }
}

void SNIRF::ParseStims(const HighFive::Group& nirs)
{
    using namespace NIRS::Events;

	size_t max_stims = 1000; // Arbitrary large number to avoid infinite loops
    for (size_t i = 1; i < max_stims; i++)
    {
        std::string stim_name = "stim" + std::to_string(i);
        if (!nirs.exist(stim_name))
            break;

        auto stim = nirs.getGroup(stim_name);

        Event event;

        // 1. Name
        std::string name;
        stim.getDataSet("name").read(name);

        std::vector<std::vector<double>> data;
        try {
            stim.getDataSet("data").read(data);
        }
        catch (const HighFive::Exception& e) {
            NVIZ_ERROR("Failed to read 'data' for {}: {}", stim_name, e.what());
            continue; // Skip this stimulus group
        }

		event.name = name;


        // 2. Markers
        event.markers.reserve(data.size());

        for (const auto& row : data) {

            if (row.size() < 3) {
                NVIZ_ERROR("Stim '{}' data row is malformed ({} columns). Skipping row.", name, row.size());
                continue;
            }

            event.markers.emplace_back(EventMarker{
                row[0], // onset
                row[1], // duration
                row[2]  // value
                });
        }

        std::sort(event.markers.begin(), event.markers.end(), [](const EventMarker& a, const EventMarker& b) {
            return a.onset < b.onset;
        });

		// 3. Data Labels (optional)
        if (stim.exist("dataLabels")) {
            std::vector<std::string> dataLabels;
            try {
                stim.getDataSet("dataLabels").read(dataLabels);
                for (const auto& label : dataLabels)
                {
                    NVIZ_INFO("    Stim Label : {}", label);
                }
            }
            catch (const HighFive::Exception& e) {
                NVIZ_ERROR("Failed to read 'dataLabels' for {}: {}", stim_name, e.what());
            }
        }
    
		events_.push_back(std::move(event));
	}
}

