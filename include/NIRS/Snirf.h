#pragma once
#include "Core/Base.h"

#include <string>
#include <filesystem>

#include <Eigen/Dense>

#include <highfive/H5Group.hpp>

#include "NIRS/NIRS.h"

class SNIRF {
public:
	SNIRF();
	SNIRF(const std::filesystem::path& filepath);

	void LoadFile(const std::filesystem::path& filepath);

	void Print();
	void ParseProbe(const HighFive::Group& probe);


	std::string GetFilepath() { return m_Filepath.string(); };

	bool IsFileLoaded() { return !m_Filepath.empty(); };


	std::vector<NIRS::Probe2D> GetProbes2D() { return m_Probes2D; };
	std::vector<NIRS::Probe3D> GetProbes3D() { return m_Probes3D; };
	std::vector<NIRS::Landmark> GetLandmarks() { return m_Landmarks; };
private:
	std::filesystem::path m_Filepath;

	Eigen::Matrix<double, 
		Eigen::Dynamic, 
		Eigen::Dynamic, 
		Eigen::RowMajor> m_ChannelData;

	std::vector<NIRS::Probe2D> m_Probes2D;
	std::vector<NIRS::Probe3D> m_Probes3D;
	std::vector<NIRS::Landmark> m_Landmarks;
};