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

SNIRF::SNIRF()
{
}

SNIRF::SNIRF(const std::filesystem::path& filepath)
{
	LoadFile(filepath);
}

void SNIRF::LoadFile(const std::filesystem::path& filepath)
{
    if(!std::filesystem::exists(filepath)) {
        NVIZ_ERROR("File does not exist: {0}", filepath.string().c_str());
        return;
	}

    m_Filepath = filepath;
	auto file = Utils::ParseHDF5(filepath.string());

	Group root_group = file.getGroup("/");
    Group nirs = root_group.getGroup("/nirs");
    Group data1 = nirs.getGroup("data1");

	Group metadata = nirs.getGroup("metaDataTags");
    Group probe = nirs.getGroup("probe");
	ParseProbe(probe);

    Print();
}


void SNIRF::Print()
{
    NVIZ_INFO("PROBES : 2D");
    size_t print_count = std::min((size_t)10, m_Probes2D.size());
    for (size_t i = 0; i < print_count; i++)
    {
        auto& probe2D = m_Probes2D[i];
        auto& probe3D = m_Probes3D[i];
        NVIZ_INFO("    {} : ( {}, {} )", Utils::ProbeTypeToString(probe2D.Type), probe2D.Position.x, probe2D.Position.y);
        NVIZ_INFO("    {} : ( {}, {}, {} )", Utils::ProbeTypeToString(probe3D.Type), probe3D.Position.x, probe3D.Position.y, probe3D.Position.z);
    }
    NVIZ_INFO("Landmarks : {}", m_Landmarks.size());
    print_count = std::min((size_t)10, m_Landmarks.size());
    for (size_t i = 0; i < print_count; i++)
    {
        auto& lm = m_Landmarks[i];
        NVIZ_INFO("    {} : ( {}, {}, {} )", lm.Name, lm.Position.x, lm.Position.y, lm.Position.z);
    }
}

void SNIRF::ParseProbe(const HighFive::Group& probe)
{
    NVIZ_INFO("PROBE : ");
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

            m_Probes2D.push_back({ glm::vec2(x, y), DETECTOR });
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

            m_Probes3D.push_back({ glm::vec3(x, z, y), DETECTOR });
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

            m_Probes2D.push_back({ glm::vec2(x, y), SOURCE });
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

            m_Probes3D.push_back({ glm::vec3(x, z, y), SOURCE });
        }
    }

    auto wavelengths = probe.getDataSet("wavelengths");

    auto landmarkLabels = probe.getDataSet("landmarkLabels");
    auto landmarkPos3D = probe.getDataSet("landmarkPos3D");
    {
        auto label_dims = landmarkLabels.getDimensions();
        std::vector<std::string> labels(label_dims[0]);
        landmarkLabels.read(labels);

        auto dims = landmarkPos3D.getDimensions();
        auto nd_array = std::vector<double>(dims[0] * dims[1]);
        landmarkPos3D.read_raw<double>(nd_array.data());
        auto positions = Map_RM(nd_array.data(), dims[0], dims[1]);

        for (int i = 0; i < positions.rows(); i++) {

            auto row_vector = positions.row(i);

            // Process the data for the i-th detector
            double x = row_vector(0);
            double y = row_vector(1);
            double z = row_vector(2);

            m_Landmarks.push_back({ labels[i], { x, y, z} });
        }
    }
}
