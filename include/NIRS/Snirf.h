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
	void ParseData1(const HighFive::Group& data1);

	std::string GetFilepath() { return m_Filepath.string(); };

	bool IsFileLoaded() { return !m_Filepath.empty(); };


	std::vector<NIRS::Landmark> GetLandmarks() { return m_Landmarks; };


	std::vector<NIRS::Probe2D> GetSources2D() { return m_Sources2D; };
	std::vector<NIRS::Probe3D> GetSources3D() { return m_Sources3D; };

	std::vector<NIRS::Probe2D> GetDetectors2D() { return m_Detectors2D; };
	std::vector<NIRS::Probe3D> GetDetectors3D() { return m_Detectors3D; };

	NIRS::Probe2D GetDetector2D(int index) { return m_Detectors2D[index]; };
	NIRS::Probe3D GetDetector3D(int index) { return m_Detectors3D[index]; };

	NIRS::Probe2D GetSource2D(int index) { return m_Sources2D[index]; };
	NIRS::Probe3D GetSource3D(int index) { return m_Sources3D[index]; };

	std::vector<NIRS::Channel> GetChannels() { return m_Channels; };

	std::vector<int> GetWavelengths() { return m_Wavelengths; };

	int GetSourceAmount()	{ return m_Sources2D.size(); };
	int GetDetectorAmount()	{ return m_Detectors2D.size(); };
private:
	std::filesystem::path m_Filepath = std::filesystem::path("");

	Eigen::Matrix<double,
		Eigen::Dynamic,
		Eigen::Dynamic,
		Eigen::RowMajor> m_ChannelData;

	std::vector<NIRS::Probe2D> m_Sources2D	 = {};
	std::vector<NIRS::Probe2D> m_Detectors2D = {};
	std::vector<NIRS::Probe3D> m_Sources3D	 = {};
	std::vector<NIRS::Probe3D> m_Detectors3D = {};
	std::vector<NIRS::Landmark> m_Landmarks	 = {};
	std::vector<NIRS::Channel> m_Channels	 = {};
	std::vector<int> m_Wavelengths			 = {};
};