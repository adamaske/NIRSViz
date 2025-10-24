#include "pch.h"
#include "NIRS/Snirf.h"

#include <HighFive/H5File.hpp>
#include <highfive/H5DataSet.hpp>
#include <highfive/H5DataSpace.hpp>
#include <highfive/H5Easy.hpp>

using namespace HighFive;
using namespace NIRS;

namespace Utils {
    using namespace HighFive;
    using namespace NIRS;

    std::string ProbeTypeToString(ProbeType type) {
        switch (type) {
        case SOURCE: return "SOURCE";
        case DETECTOR: return "DETECTOR";
        }
        return "INVALID";
    }

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

ChannelDataRegistry* ChannelDataRegistry::s_Instance = nullptr;

SNIRF::SNIRF()
{
}

SNIRF::SNIRF(const std::filesystem::path& filepath)
{
	m_ChannelDataRegistry = ChannelDataRegistry();
	LoadFile(filepath);
}



void SNIRF::Print()
{
    NVIZ_INFO("SNIRF File       : {}", m_Filepath.string());
    NVIZ_INFO("     Sources     : {}", m_Sources2D.size());
    NVIZ_INFO("     Detectors   : {}", m_Detectors2D.size());

    //NVIZ_INFO("Landmarks : 3D{}", m_Landmarks.size());
    //auto print_count = std::min((size_t)3, m_Landmarks.size());
    //for (size_t i = 0; i < print_count; i++)
    //{
    //    auto& lm = m_Landmarks[i];
    //    NVIZ_INFO("    {} : ( {}, {}, {} )", lm.Name, lm.Position.x, lm.Position.y, lm.Position.z);
    //}

    NVIZ_INFO("Wavelengths : {}, {}", m_Wavelengths[0], m_Wavelengths[1]);

    NVIZ_INFO("Channel Data : {} channels, {} time points", m_ChannelData.rows(), m_ChannelData.cols());
}

void SNIRF::LoadFile(const std::filesystem::path& filepath)
{
    if(!std::filesystem::exists(filepath)) {
        NVIZ_ERROR("File does not exist: {0}", filepath.string().c_str());
        return;
	}
    m_Sources2D.clear();
    m_Detectors2D.clear();
    m_Sources3D.clear();
    m_Detectors3D.clear();
    //m_Landmarks.clear();
    m_Channels.clear();
    m_Wavelengths.clear();
    m_ChannelData.resize(0, 0);

    m_Filepath = filepath;
    File file(filepath.string(), File::ReadOnly); //Utils::ParseHDF5(filepath.string());

	Group root_group = file.getGroup("/");
    Group nirs = root_group.getGroup("/nirs");
    Group data1 = nirs.getGroup("data1");

	Group metadata = nirs.getGroup("metaDataTags");
    Group probe = nirs.getGroup("probe");

    ParseMetadataTags(metadata);
	ParseProbe(probe); // THIS MUST BE FIRST
    ParseData1(data1);

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
    std::vector<std::string> object_names = probe.listObjectNames();

    using Map_RM = Eigen::Map<const Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>>;

    auto detectorPos2D = probe.getDataSet("detectorPos2D");
    {
        auto dims = detectorPos2D.getDimensions();
        auto nd_array = std::vector<double>(dims[0] * dims[1]);
        detectorPos2D.read_raw<double>(nd_array.data());
        auto detectors = Map_RM(nd_array.data(), dims[0], dims[1]);

        for (int i = 0; i < detectors.rows(); i++) {
            auto row_vector = detectors.row(i);

            // Process the data for the i-th detector
            double x = row_vector(0);
            double y = row_vector(1);

            m_Detectors2D.push_back({ glm::vec2(x, y), DETECTOR });
        }
    }
    auto detectorPos3D = probe.getDataSet("detectorPos3D"); 
    {
        auto dims = detectorPos3D.getDimensions();
        auto nd_array = std::vector<double>(dims[0] * dims[1]);
        detectorPos3D.read_raw<double>(nd_array.data());
        auto detectors = Map_RM(nd_array.data(), dims[0], dims[1]);

        for (int i = 0; i < detectors.rows(); i++) {
            auto row_vector = detectors.row(i);

            // Process the data for the i-th detector
            double x = row_vector(0);
            double y = row_vector(1);
            double z = row_vector(2);

            m_Detectors3D.push_back({ glm::vec3(x, z, y), DETECTOR });
        }
    }

    auto sourcePos2D = probe.getDataSet("sourcePos2D");
    {
        auto dims = sourcePos2D.getDimensions();
        auto nd_array = std::vector<double>(dims[0] * dims[1]);
        sourcePos2D.read_raw<double>(nd_array.data());
        auto detectors = Map_RM(nd_array.data(), dims[0], dims[1]);

        for (int i = 0; i < detectors.rows(); i++) {
            auto row_vector = detectors.row(i);

            // Process the data for the i-th detector
            double x = row_vector(0);
            double y = row_vector(1);

            m_Sources2D.push_back({ glm::vec2(x, y), SOURCE });
        }
    }
    auto sourcePos3D = probe.getDataSet("sourcePos3D"); 
    {
        auto dims = sourcePos3D.getDimensions();
        auto nd_array = std::vector<double>(dims[0] * dims[1]);
        sourcePos3D.read_raw<double>(nd_array.data());
        auto detectors = Map_RM(nd_array.data(), dims[0], dims[1]);

        for (int i = 0; i < detectors.rows(); i++) {
            auto row_vector = detectors.row(i);

            // Process the data for the i-th detector
            double x = row_vector(0);
            double y = row_vector(1);
            double z = row_vector(2);

            m_Sources3D.push_back({ glm::vec3(x, z, y), SOURCE });
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

    DataSet time = data1.getDataSet("time");
    {
        std::vector<double> time_data(time.getDimensions()[0]);
        time.read(time_data);
		m_Time.resize(time_data.size());
        for (size_t i = 0; i < time_data.size(); i++)
        {
            m_Time[i] = time_data[i] * 10;
        }
        float total_duration = time_data.back() - time_data.front();
        size_t num_intervals = time_data.size() - 1;
        float avg_dt = total_duration / num_intervals;
        float sampling_rate = 1.0f / avg_dt;
        m_SamplingRate = sampling_rate;
		m_DurationSeconds = total_duration;
        NVIZ_INFO("Sampling Rate (Fs): {} Hz", sampling_rate);
        NVIZ_INFO("Duration (Seconds): {} ", total_duration);
    }


    using Map_RM = Eigen::Map<const Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>>;
    auto dataTimeSeries = data1.getDataSet("dataTimeSeries");
    {
        auto dims = dataTimeSeries.getDimensions();
        auto nd_array = std::vector<double>(dims[0] * dims[1]);
        dataTimeSeries.read_raw<double>(nd_array.data());
        m_ChannelData = Map_RM(nd_array.data(), dims[0], dims[1]).transpose();
	}

	std::string base_name = "measurementList";
    for (size_t i = 1; i < m_ChannelData.rows() + 1; i++)
    {
		auto name = base_name + std::to_string(i);

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
        
		NIRS::Channel channel;
		channel.ID = i; // Hopefully this is fine? We shouldnt actually care about this ID, its just for us to identify channels internally
		channel.SourceID = sourceIndex;
		channel.DetectorID = detectorIndex;
        
        if (dataTypeIndex == -1) {
            // Parse dataTypeLabel
            if(dataTypeLabel == "HbO") {
                channel.Wavelength = NIRS::WavelengthType::HBO;
            }
            else if (dataTypeLabel == "HbR") {
                channel.Wavelength = NIRS::WavelengthType::HBR;
            }
            else if (dataTypeLabel == "HbT") {
                channel.Wavelength = NIRS::WavelengthType::HBT;
            }
            else {
                NVIZ_ERROR("Unknown dataTypeLabel: {0}", dataTypeLabel);
                channel.Wavelength = NIRS::WavelengthType::HBR; // Default to something
			}
        }
        else {
            channel.Wavelength = NIRS::WavelengthType(wavelengthIndex - 1);
        }


        std::vector<double> channel_data_vec;
        auto channel_row = m_ChannelData.row(i - 1);
        channel_data_vec.resize(m_ChannelData.cols());
        std::copy(channel_row.data(), channel_row.data() + channel_row.size(), channel_data_vec.begin());

		channel.DataIndex = m_ChannelDataRegistry.SubmitChannelData(channel_data_vec);

		m_Channels.push_back(channel);
        if (i == 1) {
            NVIZ_INFO("Measurement List : {0}", name);
            NVIZ_INFO("    dataType        : {0}", dataType);
            NVIZ_INFO("    dataTypeIndex   : {0}", dataTypeIndex);
            NVIZ_INFO("    dataTypeLabel   : {0}", dataTypeLabel); // Either raw-DC, or conc or something else
            NVIZ_INFO("Channel : ");
            NVIZ_INFO("    Source ID     : {0}", channel.SourceID);
            NVIZ_INFO("    Detector ID   : {0}", channel.DetectorID);
            NVIZ_INFO("    Wavelength    : {0}", NIRS::WavelengthTypeToString(channel.Wavelength));
            NVIZ_INFO("    Data          : {0}", channel.DataIndex);
        }
    }
}

