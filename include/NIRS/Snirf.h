#pragma once
#include "Core/Base.h"

#include <string>
#include <filesystem>

#include <Eigen/Dense>

#include <highfive/H5Group.hpp>

enum ProbeType {
	SOURCE,
	DETECTOR
};

struct Probe2D {
	glm::vec2 Position;
	ProbeType Type;
};
struct Probe3D {
	glm::vec3 Position;
	ProbeType Type;
};

struct Landmark {
	std::string Name;
	glm::vec3 Position;
};

class SNIRF {
public:
	SNIRF();
	SNIRF(const std::filesystem::path& filepath);

	void LoadFile(const std::filesystem::path& filepath);

	void Print();
	void ParseProbe(const HighFive::Group& probe);


	std::string GetFilepath() { return m_Filepath.string(); };

	bool IsFileLoaded() { return !m_Filepath.empty(); };


	std::vector<Probe2D> GetProbes2D() { return m_Probes2D; };
	std::vector<Probe3D> GetProbes3D() { return m_Probes3D; };
	std::vector<Landmark> GetLandmarks() { return m_Landmarks; };
private:
	std::filesystem::path m_Filepath;

	Eigen::Matrix<double, 
		Eigen::Dynamic, 
		Eigen::Dynamic, 
		Eigen::RowMajor> m_ChannelData;

	std::vector<Probe2D> m_Probes2D;
	std::vector<Probe3D> m_Probes3D;
	std::vector<Landmark> m_Landmarks;
};